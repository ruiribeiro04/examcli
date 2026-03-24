#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "ArgParser.hpp"
#include "SubjectLoader.hpp"
#include "FileReader.hpp"
#include "LLMClient.hpp"
#include "Result.hpp"
#include "Generator.hpp"
#include "Validator.hpp"
#include "QualityScorer.hpp"
#include "ExerciseSaver.hpp"
#include "ExerciseExporter.hpp"
#include "ExerciseImporter.hpp"

bool readMetadata(const std::string &exercisePath, std::string &metadataContent) {
    std::string metadataPath = exercisePath + "/metadata.json";
    std::ifstream file(metadataPath.c_str());

    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    metadataContent = buffer.str();
    return true;
}

bool writeMetadata(const std::string &exercisePath, const std::string &metadataContent) {
    std::string metadataPath = exercisePath + "/metadata.json";
    std::ofstream file(metadataPath.c_str());

    if (!file.is_open()) {
        return false;
    }

    file << metadataContent;
    file.close();

    return true;
}

std::string findExercisePath(const std::string &exerciseName) {
    std::string generatedPath = "subjects/generated";
    std::string ranks[] = {"rank02", "rank03", "rank04", "rank05"};

    for (size_t i = 0; i < sizeof(ranks)/sizeof(ranks[0]); ++i) {
        std::string path = generatedPath + "/" + ranks[i] + "/" + exerciseName;
        if (access(path.c_str(), F_OK) == 0) {
            return path;
        }
    }

    return "";
}

int main(int argc, char **argv) {
    ArgParser parser;

    if (!parser.parse(argc, argv)) {
        return 1;
    }

    if (parser.shouldShowHelp()) {
        return 0;
    }

    Command cmd = parser.getCommand();

    if (cmd == CMD_GENERATE) {
        std::string rank = parser.hasRank() ? parser.getRank() : "rank02";
        std::string level = parser.hasLevel() ? parser.getLevel() : "level0";
        std::string prompt = parser.getPrompt();

        if (parser.verbose()) {
            std::cout << "[VERBOSE] Generating exercise with rank=" << rank
                      << ", level=" << level << ", prompt=" << prompt << "\n";
        }

        Validator::setupSignalHandler();
        if (!parser.verbose()) {
            std::cout << "Press Ctrl+C to cancel generation (will finish current stage)\n" << std::endl;
        }

        Generator generator;
        if (!generator.initialize(rank, level, "function")) {
            std::cerr << "Error: " << generator.getLastError() << "\n";
            return 1;
        }

        ExerciseData exercise;
        if (!generator.generate(prompt, exercise)) {
            std::cerr << "Error: " << generator.getLastError() << "\n";
            return 1;
        }

        std::cout << "Exercise generated successfully\n";

        Validator validator;
        ValidationResult validation;
        if (!validator.validate(exercise, validation)) {
            if (validation.cancelled) {
                std::cout << "Generation cancelled by user\n";
                return 130;
            }
            std::cerr << "Validation failed: " << validation.message << "\n";
            return 1;
        }

        std::cout << "Exercise validated successfully\n";

        QualityScorer scorer;
        QualityReport report;
        if (!scorer.calculateQuality(exercise, validator.getCompilationLog(), report)) {
            std::cerr << "Error: " << scorer.getLastError() << "\n";
            return 1;
        }

        if (parser.verbose()) {
            std::cout << "[VERBOSE] Quality Report:\n";
            std::cout << "  Overall: " << report.overall_score << "/100\n";
            std::cout << "  Compilation: " << report.compilation_score << "/100\n";
            std::cout << "  Test Coverage: " << report.test_coverage_score << "/100\n";
            std::cout << "  Subject Clarity: " << report.subject_clarity_score << "/100\n";
            std::cout << "  Code Quality: " << report.code_quality_score << "/100\n";
        }

        std::cout << "\nQuality Report:\n";
        std::cout << "  Overall Score: " << report.overall_score << "/100\n";
        std::cout << "  Status: " << (report.approved ? "APPROVED" : "REJECTED") << "\n\n";

        if (!report.approved) {
            std::cerr << "Exercise quality below threshold (70/100). Not saving.\n";
            return 1;
        }

        ExerciseSaver saver;
        if (!saver.saveExercise(exercise, report, rank, level, prompt,
                               validator.getCompilationLog(),
                               validator.getTestResults(),
                               "gpt-4", 1)) {
            std::cerr << "Error: " << saver.getLastError() << "\n";
            return 1;
        }

        std::cout << "\nTry your exercise:\n";
        std::cout << "  ./examcli practice -r " << rank << " -l " << level
                  << " -s " << exercise.exercise_name << " your_solution.c\n";

        return 0;
    }

    if (cmd == CMD_SHARE) {
        std::string exerciseName = parser.getExerciseName();
        std::string outputPath = parser.getOutputPath();

        if (outputPath.empty()) {
            outputPath = exerciseName + (parser.metadataOnly() ? "_metadata.json" : ".tar.gz");
        }

        if (parser.verbose()) {
            std::cout << "[VERBOSE] Sharing exercise: " << exerciseName
                      << " to " << outputPath << "\n";
        }

        ExerciseExporter exporter;
        if (parser.metadataOnly()) {
            if (!exporter.exportMetadataOnly(exerciseName, outputPath)) {
                std::cerr << "Error: " << exporter.getLastError() << "\n";
                return 1;
            }
        } else {
            if (!exporter.exportExercise(exerciseName, outputPath)) {
                std::cerr << "Error: " << exporter.getLastError() << "\n";
                return 1;
            }
        }

        std::cout << "Exercise shared successfully: " << outputPath << "\n";
        return 0;
    }

    if (cmd == CMD_IMPORT) {
        std::string importFile = parser.getImportFile();
        std::string rankOverride = parser.hasRank() ? parser.getRank() : "";

        if (parser.verbose()) {
            std::cout << "[VERBOSE] Importing: " << importFile
                      << " with rank override: " << rankOverride << "\n";
        }

        ExerciseImporter importer;
        if (!importer.importExercise(importFile, rankOverride)) {
            std::cerr << "Error: " << importer.getLastError() << "\n";
            return 1;
        }

        std::cout << "Exercise imported successfully\n";
        return 0;
    }

    if (cmd == CMD_LIST) {
        std::string subcommand = parser.getListSubcommand();
        SubjectLoader loader;

        if (subcommand == "ranks") {
            loader.showAvailableRanks();
            return 0;
        } else if (subcommand == "levels") {
            if (!parser.hasRank()) {
                std::cerr << "Error: 'list levels' requires a rank (-r)\n";
                return 1;
            }
            loader.showAvailableLevels(parser.getRank());
            return 0;
        } else if (subcommand == "subjects") {
            if (!parser.hasRank() || !parser.hasLevel()) {
                std::cerr << "Error: 'list subjects' requires both rank (-r) and level (-l)\n";
                return 1;
            }
            loader.showAvailableSubjects(parser.getRank(), parser.getLevel());
            return 0;
        } else if (subcommand == "generated") {
            std::cout << "Generated exercises:\n";
            std::cout << "  (Listing not yet implemented - check subjects/generated/)\n";
            return 0;
        } else if (subcommand == "shareable") {
            std::cout << "Shareable exercises:\n";
            std::cout << "  (Listing not yet implemented - check subjects/generated/)\n";
            return 0;
        } else {
            std::cerr << "Error: Invalid list subcommand: " << subcommand << "\n";
            std::cerr << "Hint: Valid subcommands are: ranks, levels, subjects, generated, shareable\n";
            return 1;
        }
    }

    if (cmd == CMD_RATE) {
        std::string exerciseName = parser.getExerciseName();
        std::string scoreStr = parser.getRatingScore();

        int score = atoi(scoreStr.c_str());
        if (score < 1 || score > 5) {
            std::cerr << "Error: Rating must be between 1 and 5\n";
            return 1;
        }

        std::string exercisePath = findExercisePath(exerciseName);
        if (exercisePath.empty()) {
            std::cerr << "Error: Exercise not found: " << exerciseName << "\n";
            std::cerr << "Hint: Use 'examcli list generated' to see available exercises\n";
            return 1;
        }

        std::string metadataContent;
        if (!readMetadata(exercisePath, metadataContent)) {
            std::cerr << "Error: Could not read metadata.json for exercise\n";
            return 1;
        }

        size_t ratingsPos = metadataContent.find("\"ratings\":");
        if (ratingsPos == std::string::npos) {
            std::cerr << "Error: Invalid metadata.json format (missing ratings)\n";
            return 1;
        }

        const char *username = getenv("USER");
        if (!username) {
            username = getenv("USERNAME");
        }
        if (!username) {
            username = "user";
        }

        std::string userSearch = "\"user\": \"" + std::string(username) + "\"";
        size_t userPos = metadataContent.find(userSearch, ratingsPos);

        if (userPos != std::string::npos) {
            size_t scoreStart = metadataContent.find("\"score\":", userPos);
            if (scoreStart != std::string::npos) {
                size_t scoreEnd = metadataContent.find("}", scoreStart);
                if (scoreEnd != std::string::npos) {
                    std::string before = metadataContent.substr(0, scoreStart + 9);
                    std::string after = metadataContent.substr(scoreEnd);
                    metadataContent = before + " " + scoreStr + after;
                }
            }
            std::cout << "Updated your rating for '" << exerciseName << "' to " << score << "/5\n";
        } else {
            size_t arrayEnd = metadataContent.find("]", ratingsPos);
            if (arrayEnd == std::string::npos) {
                std::cerr << "Error: Invalid metadata.json format (unclosed ratings array)\n";
                return 1;
            }

            size_t arrayStart = metadataContent.find("[", ratingsPos);
            std::string arrayContent = metadataContent.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
            bool isEmpty = (arrayContent.find_first_not_of(" \t\n\r") == std::string::npos);

            std::string newRating = "{\"user\": \"" + std::string(username) + "\", \"score\": " + scoreStr + "}";

            std::string before = metadataContent.substr(0, arrayEnd);
            std::string after = metadataContent.substr(arrayEnd);

            if (!isEmpty) {
                metadataContent = before + ", " + newRating + after;
            } else {
                metadataContent = before + newRating + after;
            }

            std::cout << "Rated '" << exerciseName << "' with " << score << "/5\n";
        }

        int count = 0;
        int sum = 0;
        size_t pos = ratingsPos;
        while ((pos = metadataContent.find("\"score\":", pos)) != std::string::npos &&
               pos < metadataContent.find("]", ratingsPos)) {
            size_t valueStart = pos + 9;
            size_t valueEnd = metadataContent.find_first_of(",}", valueStart);
            if (valueEnd != std::string::npos) {
                std::string valueStr = metadataContent.substr(valueStart, valueEnd - valueStart);
                size_t start = valueStr.find_first_not_of(" ");
                size_t end = valueStr.find_last_not_of(" ");
                if (start != std::string::npos) {
                    valueStr = valueStr.substr(start, end - start + 1);
                    sum += atoi(valueStr.c_str());
                    count++;
                }
            }
            pos = valueEnd;
        }

        double avg = count > 0 ? (double)sum / count : 0.0;

        size_t avgPos = metadataContent.find("\"average_rating\":");
        if (avgPos != std::string::npos) {
            size_t avgEnd = metadataContent.find(",", avgPos);
            if (avgEnd == std::string::npos) {
                avgEnd = metadataContent.find("\n", avgPos);
            }
            if (avgEnd != std::string::npos) {
                std::string before = metadataContent.substr(0, avgPos + 17);
                std::string after = metadataContent.substr(avgEnd);

                char avgStr[20];
                sprintf(avgStr, "%.2f", avg);

                metadataContent = before + " " + avgStr + after;
            }
        }

        size_t countPos = metadataContent.find("\"rating_count\":");
        if (countPos != std::string::npos) {
            size_t countEnd = metadataContent.find(",", countPos);
            if (countEnd == std::string::npos) {
                countEnd = metadataContent.find("\n", countPos);
            }
            if (countEnd != std::string::npos) {
                std::string before = metadataContent.substr(0, countPos + 15);
                std::string after = metadataContent.substr(countEnd);
                metadataContent = before + " " + scoreStr + after;
            }
        }

        if (!writeMetadata(exercisePath, metadataContent)) {
            std::cerr << "Error: Could not write metadata.json\n";
            return 1;
        }

        std::cout << "Average rating: " << avg << "/5 (" << count << " ratings)\n";
        return 0;
    }

    if (cmd == CMD_INFO) {
        std::string exerciseName = parser.getExerciseName();

        std::string exercisePath = findExercisePath(exerciseName);
        if (exercisePath.empty()) {
            std::cerr << "Error: Exercise not found: " << exerciseName << "\n";
            std::cerr << "Hint: Use 'examcli list generated' to see available exercises\n";
            return 1;
        }

        std::string metadataContent;
        if (!readMetadata(exercisePath, metadataContent)) {
            std::cerr << "Error: Could not read metadata.json for exercise\n";
            return 1;
        }

        std::cout << "\nExercise Information: " << exerciseName << "\n\n";

        size_t pos = metadataContent.find("\"exercise_name\":");
        if (pos != std::string::npos) {
            size_t start = metadataContent.find("\"", pos + 16) + 1;
            size_t end = metadataContent.find("\"", start);
            if (end != std::string::npos) {
                std::cout << "Name: " << metadataContent.substr(start, end - start) << "\n";
            }
        }

        pos = metadataContent.find("\"rank\":");
        if (pos != std::string::npos) {
            size_t start = metadataContent.find("\"", pos + 7) + 1;
            size_t end = metadataContent.find("\"", start);
            if (end != std::string::npos) {
                std::cout << "Rank: " << metadataContent.substr(start, end - start) << "\n";
            }
        }

        pos = metadataContent.find("\"level\":");
        if (pos != std::string::npos) {
            size_t start = metadataContent.find("\"", pos + 8) + 1;
            size_t end = metadataContent.find("\"", start);
            if (end != std::string::npos) {
                std::cout << "Level: " << metadataContent.substr(start, end - start) << "\n";
            }
        }

        std::cout << "\nLocation: " << exercisePath << "\n\n";
        return 0;
    }

    if (cmd == CMD_PRACTICE) {
        std::string rank = parser.hasRank() ? parser.getRank() : "rank02";
        std::string level = parser.hasLevel() ? parser.getLevel() : "level0";
        std::string subject = parser.hasSubject() ? parser.getSubject() : "fizzbuzz";
        std::string sourceFile = parser.getSourceFile();

        if (parser.verbose()) {
            std::cout << "[VERBOSE] Practice mode: rank=" << rank
                      << ", level=" << level << ", subject=" << subject
                      << ", file=" << sourceFile << "\n";
        }

        SubjectLoader loader;
        if (!loader.load(rank, level, subject)) {
            return 1;
        }

        FileReader reader;
        std::string code = reader.read(sourceFile);

        if (code.empty()) {
            std::cerr << "Error: Could not read source file: " << sourceFile << "\n";
            return 1;
        }

        LLMClient client;
        if (!client.initialize()) {
            std::cerr << "Error: " << client.getLastError() << "\n";
            return 1;
        }

        bool isCorrect = false;
        std::string hint;

        std::cout << "Submitting code for correction...\n";

        if (!client.correct(loader.getSubjectContent(), code, isCorrect, hint)) {
            std::cerr << "Error: " << client.getLastError() << "\n";
            return 1;
        }

        Result result;
        result.setCorrectionResult(isCorrect, hint);
        result.show();

        return result.getExitCode();
    }

    parser.showUsage();
    return 1;
}
