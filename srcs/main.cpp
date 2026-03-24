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

// Helper function to read metadata.json
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

// Helper function to write metadata.json
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

// Helper function to find exercise path
std::string findExercisePath(const std::string &exerciseName) {
    // Check in subjects/generated/ first
    std::string generatedPath = "subjects/generated";

    // Try different ranks
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

    // Handle GENERATE command
    if (cmd == CMD_GENERATE) {
        std::string rank = parser.hasRank() ? parser.getRank() : "rank02";
        std::string level = parser.hasLevel() ? parser.getLevel() : "level0";
        std::string prompt = parser.getPrompt();
        std::string type = "function"; // Default type

        // Setup signal handler for Ctrl+C cancellation
        Validator::setupSignalHandler();
        std::cout << "Press Ctrl+C to cancel generation (will finish current stage)\n" << std::endl;

        Generator generator;
        if (!generator.initialize(rank, level, type)) {
            std::cerr << "Error: " << generator.getLastError() << "\n";
            return 1;
        }

        ExerciseData exercise;
        if (!generator.generate(prompt, exercise)) {
            std::cerr << "Error: " << generator.getLastError() << "\n";
            return 1;
        }

        std::cout << "✓ Exercise generated successfully\n";

        // Validate the exercise
        Validator validator;
        ValidationResult validation;
        if (!validator.validate(exercise, validation)) {
            if (validation.cancelled) {
                std::cout << "\n⚠️  Generation cancelled by user\n";
                return 130; // Standard exit code for SIGINT
            }
            std::cerr << "Validation failed: " << validation.message << "\n";
            return 1;
        }

        std::cout << "✓ Exercise validated successfully\n";

        // Score the exercise
        QualityScorer scorer;
        QualityReport report;
        if (!scorer.calculateQuality(exercise, validator.getCompilationLog(), report)) {
            std::cerr << "Error: " << scorer.getLastError() << "\n";
            return 1;
        }

        // Display quality report
        std::cout << "\n📊 Quality Report:\n";
        std::cout << "  Overall Score: " << report.overall_score << "/100\n";
        std::cout << "  Compilation: " << report.compilation_score << "/100\n";
        std::cout << "  Test Coverage: " << report.test_coverage_score << "/100\n";
        std::cout << "  Subject Clarity: " << report.subject_clarity_score << "/100\n";
        std::cout << "  Code Quality: " << report.code_quality_score << "/100\n";
        std::cout << "\n  Status: " << (report.approved ? "✅ APPROVED" : "❌ REJECTED") << "\n\n";

        if (!report.approved) {
            std::cerr << "Exercise quality below threshold (70/100). Not saving.\n";
            return 1;
        }

        // Save the exercise
        ExerciseSaver saver;
        if (!saver.saveExercise(exercise, report, rank, level, prompt,
                               validator.getCompilationLog(),
                               validator.getTestResults(),
                               "gpt-4", 1)) {
            std::cerr << "Error: " << saver.getLastError() << "\n";
            return 1;
        }

        std::cout << "\n💡 Try your exercise:\n";
        std::cout << "  ./examcli -r " << rank << " -l " << level
                  << " -s " << exercise.exercise_name << " your_solution.c\n";

        return 0;
    }

    // Handle SHARE command
    if (cmd == CMD_SHARE) {
        std::string exerciseName = parser.getExerciseName();
        std::string outputPath = parser.getOutputPath();

        if (outputPath.empty()) {
            outputPath = exerciseName + (parser.metadataOnly() ? "_metadata.json" : ".tar.gz");
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

        return 0;
    }

    // Handle IMPORT command
    if (cmd == CMD_IMPORT) {
        std::string importFile = parser.getImportFile();
        std::string rankOverride = parser.hasRank() ? parser.getRank() : "";

        ExerciseImporter importer;
        if (!importer.importExercise(importFile, rankOverride)) {
            std::cerr << "Error: " << importer.getLastError() << "\n";
            return 1;
        }

        return 0;
    }

    // Handle LIST command with --generated or --shareable flags
    if (cmd == CMD_LIST) {
        if (parser.shouldListGenerated()) {
            std::cout << "Generated exercises:\n";
            std::cout << "  (Listing not yet implemented - check subjects/generated/)\n";
            return 0;
        }
        if (parser.shouldListShareable()) {
            std::cout << "Shareable exercises:\n";
            std::cout << "  (Listing not yet implemented - check subjects/generated/)\n";
            return 0;
        }
    }

    // Handle RATE command
    if (cmd == CMD_RATE) {
        std::string exerciseName = parser.getExerciseName();
        std::string scoreStr = parser.getRatingScore();

        // Validate score range
        int score = atoi(scoreStr.c_str());
        if (score < 1 || score > 5) {
            std::cerr << "Error: Rating must be between 1 and 5\n";
            return 1;
        }

        // Find exercise path
        std::string exercisePath = findExercisePath(exerciseName);
        if (exercisePath.empty()) {
            std::cerr << "Error: Exercise not found: " << exerciseName << "\n";
            std::cerr << "Hint: Use 'list --generated' to see available exercises\n";
            return 1;
        }

        // Read current metadata
        std::string metadataContent;
        if (!readMetadata(exercisePath, metadataContent)) {
            std::cerr << "Error: Could not read metadata.json for exercise\n";
            return 1;
        }

        // Simple JSON parsing to extract ratings array
        // Find the ratings array
        size_t ratingsPos = metadataContent.find("\"ratings\":");
        if (ratingsPos == std::string::npos) {
            std::cerr << "Error: Invalid metadata.json format (missing ratings)\n";
            return 1;
        }

        // Get current username (fallback to "user" if not available)
        const char *username = getenv("USER");
        if (!username) {
            username = getenv("USERNAME");
        }
        if (!username) {
            username = "user";
        }

        // Check if user already rated
        std::string userSearch = "\"user\": \"" + std::string(username) + "\"";
        size_t userPos = metadataContent.find(userSearch, ratingsPos);

        if (userPos != std::string::npos) {
            // User already rated - update existing rating
            size_t scoreStart = metadataContent.find("\"score\":", userPos);
            if (scoreStart != std::string::npos) {
                size_t scoreEnd = metadataContent.find("}", scoreStart);
                if (scoreEnd != std::string::npos) {
                    // Replace the score
                    std::string before = metadataContent.substr(0, scoreStart + 9); // +9 for "score":
                    std::string after = metadataContent.substr(scoreEnd);
                    metadataContent = before + " " + scoreStr + after;
                }
            }
            std::cout << "Updated your rating for '" << exerciseName << "' to " << score << "/5\n";
        } else {
            // Add new rating
            // Find end of ratings array
            size_t arrayEnd = metadataContent.find("]", ratingsPos);
            if (arrayEnd == std::string::npos) {
                std::cerr << "Error: Invalid metadata.json format (unclosed ratings array)\n";
                return 1;
            }

            // Check if array is empty
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

        // Recalculate average and count
        int count = 0;
        int sum = 0;
        size_t pos = ratingsPos;
        while ((pos = metadataContent.find("\"score\":", pos)) != std::string::npos &&
               pos < metadataContent.find("]", ratingsPos)) {
            size_t valueStart = pos + 9; // Skip "score":
            size_t valueEnd = metadataContent.find_first_of(",}", valueStart);
            if (valueEnd != std::string::npos) {
                std::string valueStr = metadataContent.substr(valueStart, valueEnd - valueStart);
                // Trim whitespace
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

        // Update average_rating and rating_count
        double average = count > 0 ? (double)sum / count : 0.0;

        // Find and replace average_rating
        size_t avgPos = metadataContent.find("\"average_rating\":");
        if (avgPos != std::string::npos) {
            size_t avgEnd = metadataContent.find(",", avgPos);
            if (avgEnd == std::string::npos) {
                avgEnd = metadataContent.find("\n", avgPos);
            }
            if (avgEnd != std::string::npos) {
                std::string before = metadataContent.substr(0, avgPos + 17); // +17 for "average_rating":
                std::string after = metadataContent.substr(avgEnd);

                // Format average to 2 decimal places
                char avgStr[20];
                sprintf(avgStr, "%.2f", average);

                metadataContent = before + " " + avgStr + after;
            }
        }

        // Find and replace rating_count
        size_t countPos = metadataContent.find("\"rating_count\":");
        if (countPos != std::string::npos) {
            size_t countEnd = metadataContent.find(",", countPos);
            if (countEnd == std::string::npos) {
                countEnd = metadataContent.find("\n", countPos);
            }
            if (countEnd != std::string::npos) {
                std::string before = metadataContent.substr(0, countPos + 15); // +15 for "rating_count":
                std::string after = metadataContent.substr(countEnd);
                metadataContent = before + " " + scoreStr + after;
            }
        }

        // Write updated metadata
        if (!writeMetadata(exercisePath, metadataContent)) {
            std::cerr << "Error: Could not write metadata.json\n";
            return 1;
        }

        std::cout << "Average rating: " << average << "/5 (" << count << " ratings)\n";
        return 0;
    }

    // Handle INFO command
    if (cmd == CMD_INFO) {
        std::string exerciseName = parser.getExerciseName();

        // Find exercise path
        std::string exercisePath = findExercisePath(exerciseName);
        if (exercisePath.empty()) {
            std::cerr << "Error: Exercise not found: " << exerciseName << "\n";
            std::cerr << "Hint: Use 'list --generated' to see available exercises\n";
            return 1;
        }

        // Read metadata
        std::string metadataContent;
        if (!readMetadata(exercisePath, metadataContent)) {
            std::cerr << "Error: Could not read metadata.json for exercise\n";
            return 1;
        }

        // Parse and display key information
        std::cout << "\n📋 Exercise Information: " << exerciseName << "\n\n";

        // Extract exercise_name
        size_t pos = metadataContent.find("\"exercise_name\":");
        if (pos != std::string::npos) {
            size_t start = metadataContent.find("\"", pos + 16) + 1;
            size_t end = metadataContent.find("\"", start);
            if (end != std::string::npos) {
                std::cout << "Name: " << metadataContent.substr(start, end - start) << "\n";
            }
        }

        // Extract rank and level
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

        pos = metadataContent.find("\"exercise_type\":");
        if (pos != std::string::npos) {
            size_t start = metadataContent.find("\"", pos + 16) + 1;
            size_t end = metadataContent.find("\"", start);
            if (end != std::string::npos) {
                std::cout << "Type: " << metadataContent.substr(start, end - start) << "\n";
            }
        }

        // Extract generated_at
        pos = metadataContent.find("\"generated_at\":");
        if (pos != std::string::npos) {
            size_t start = metadataContent.find("\"", pos + 15) + 1;
            size_t end = metadataContent.find("\"", start);
            if (end != std::string::npos) {
                std::cout << "Generated: " << metadataContent.substr(start, end - start) << "\n";
            }
        }

        // Extract LLM model
        pos = metadataContent.find("\"llm_model\":");
        if (pos != std::string::npos) {
            size_t start = metadataContent.find("\"", pos + 12) + 1;
            size_t end = metadataContent.find("\"", start);
            if (end != std::string::npos) {
                std::cout << "Model: " << metadataContent.substr(start, end - start) << "\n";
            }
        }

        // Extract quality scores
        pos = metadataContent.find("\"quality_scores\":");
        if (pos != std::string::npos) {
            std::cout << "\n📊 Quality Scores:\n";

            // Overall
            size_t overallPos = metadataContent.find("\"overall\":", pos);
            if (overallPos != std::string::npos) {
                size_t end = metadataContent.find(",", overallPos);
                std::string score = metadataContent.substr(overallPos + 10, end - overallPos - 10);
                std::cout << "  Overall: " << score << "/100\n";
            }

            // Compilation
            size_t compPos = metadataContent.find("\"compilation\":", pos);
            if (compPos != std::string::npos) {
                size_t end = metadataContent.find(",", compPos);
                std::string score = metadataContent.substr(compPos + 15, end - compPos - 15);
                std::cout << "  Compilation: " << score << "/100\n";
            }

            // Test coverage
            size_t testPos = metadataContent.find("\"test_coverage\":", pos);
            if (testPos != std::string::npos) {
                size_t end = metadataContent.find(",", testPos);
                std::string score = metadataContent.substr(testPos + 17, end - testPos - 17);
                std::cout << "  Test Coverage: " << score << "/100\n";
            }

            // Subject clarity
            size_t clarityPos = metadataContent.find("\"subject_clarity\":", pos);
            if (clarityPos != std::string::npos) {
                size_t end = metadataContent.find(",", clarityPos);
                std::string score = metadataContent.substr(clarityPos + 19, end - clarityPos - 19);
                std::cout << "  Subject Clarity: " << score << "/100\n";
            }

            // Code quality
            size_t qualityPos = metadataContent.find("\"code_quality\":", pos);
            if (qualityPos != std::string::npos) {
                size_t end = metadataContent.find("\n", qualityPos);
                std::string score = metadataContent.substr(qualityPos + 16, end - qualityPos - 16);
                std::cout << "  Code Quality: " << score << "/100\n";
            }
        }

        // Extract validation rounds
        pos = metadataContent.find("\"validation_rounds\":");
        if (pos != std::string::npos) {
            size_t end = metadataContent.find(",", pos);
            std::string rounds = metadataContent.substr(pos + 20, end - pos - 20);
            std::cout << "\nValidation Rounds: " << rounds << "\n";
        }

        // Extract approval status
        pos = metadataContent.find("\"approved\":");
        if (pos != std::string::npos) {
            size_t end = metadataContent.find(",", pos);
            std::string approved = metadataContent.substr(pos + 11, end - pos - 11);
            std::cout << "Status: " << (approved == "true" ? "✅ Approved" : "❌ Rejected") << "\n";
        }

        // Extract ratings
        pos = metadataContent.find("\"ratings\":");
        if (pos != std::string::npos) {
            size_t avgPos = metadataContent.find("\"average_rating\":");
            size_t countPos = metadataContent.find("\"rating_count\":");

            if (avgPos != std::string::npos && countPos != std::string::npos) {
                size_t avgEnd = metadataContent.find(",", avgPos);
                size_t countEnd = metadataContent.find(",", countPos);

                std::string avg = metadataContent.substr(avgPos + 17, avgEnd - avgPos - 17);
                std::string count = metadataContent.substr(countPos + 15, countEnd - countPos - 15);

                std::cout << "\n⭐ User Ratings:\n";
                std::cout << "  Average: " << avg << "/5\n";
                std::cout << "  Total Ratings: " << count << "\n";
            }
        }

        // Extract import info if present
        pos = metadataContent.find("\"imported_at\":");
        if (pos != std::string::npos) {
            size_t start = metadataContent.find("\"", pos + 13) + 1;
            size_t end = metadataContent.find("\"", start);
            if (end != std::string::npos) {
                std::cout << "\nImported: " << metadataContent.substr(start, end - start) << "\n";

                pos = metadataContent.find("\"imported_by\":");
                if (pos != std::string::npos) {
                    start = metadataContent.find("\"", pos + 13) + 1;
                    end = metadataContent.find("\"", start);
                    if (end != std::string::npos) {
                        std::cout << "Imported by: " << metadataContent.substr(start, end - start) << "\n";
                    }
                }
            }
        }

        std::cout << "\nLocation: " << exercisePath << "\n\n";
        return 0;
    }

    SubjectLoader loader;

    if (parser.shouldShowAll()) {
        loader.showAll();
        return 0;
    }

    if (parser.shouldListRanks()) {
        loader.showAvailableRanks();
        return 0;
    }

    if (parser.shouldListLevels()) {
        if (!parser.hasRank()) {
            std::cerr << "Error: --list-levels requires a rank (-r)\n";
            parser.showUsage();
            return 1;
        }
        loader.showAvailableLevels(parser.getRank());
        return 0;
    }

    if (parser.shouldListSubjects()) {
        if (!parser.hasRank() || !parser.hasLevel()) {
            std::cerr << "Error: --list-subjects requires a rank (-r) and level (-l)\n";
            parser.showUsage();
            return 1;
        }
        loader.showAvailableSubjects(parser.getRank(), parser.getLevel());
        return 0;
    }

    // Default: PRACTICE mode
    std::string rank = parser.hasRank() ? parser.getRank() : "rank02";
    std::string level = parser.hasLevel() ? parser.getLevel() : "level0";
    std::string subject = parser.hasSubject() ? parser.getSubject() : "fizzbuzz";
    std::string sourceFile = parser.getSourceFile();

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
