#include "ExerciseSaver.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>

// C++98 compatible to_string helper
namespace {
    template<typename T>
    std::string to_string(T value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
}

ExerciseSaver::ExerciseSaver()
    : _lastError(), _baseGeneratedPath("subjects/generated"), _lastSavedPath() {
}

ExerciseSaver::ExerciseSaver(const ExerciseSaver &other)
    : _lastError(other._lastError),
      _baseGeneratedPath(other._baseGeneratedPath),
      _lastSavedPath(other._lastSavedPath) {
}

ExerciseSaver &ExerciseSaver::operator=(const ExerciseSaver &other) {
    if (this != &other) {
        _lastError = other._lastError;
        _baseGeneratedPath = other._baseGeneratedPath;
        _lastSavedPath = other._lastSavedPath;
    }
    return *this;
}

ExerciseSaver::~ExerciseSaver() {
}

bool ExerciseSaver::createDirectory(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return true; // Directory exists and is a directory
        } else {
            _lastError = "Path exists but is not a directory: " + path;
            return false;
        }
    }

    // Create directory with parents
    std::string command = "mkdir -p \"" + path + "\" 2>&1";
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == NULL) {
        _lastError = "Failed to execute mkdir command for: " + path;
        return false;
    }

    // Check command output for errors
    char buffer[256];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        output += buffer;
    }
    int result = pclose(pipe);

    if (result != 0) {
        _lastError = "Failed to create directory '" + path + "'. System error: " + output;
        if (output.find("Permission denied") != std::string::npos) {
            _lastError += "\n💡 Tip: Check that you have write permissions for this directory.";
        } else if (output.find("Disk full") != std::string::npos ||
                   output.find("No space left") != std::string::npos) {
            _lastError += "\n💡 Tip: Your disk is full. Free up some space and try again.";
        }
        return false;
    }
    return true;
}

bool ExerciseSaver::fileExists(const std::string &path) const {
    struct stat st;
    return (stat(path.c_str(), &st) == 0);
}

bool ExerciseSaver::checkSystemScripts() const {
    // Check if .system directory exists
    if (!fileExists(".system")) {
        std::cerr << "⚠️  Warning: .system directory not found in current directory." << std::endl;
        std::cerr << "💡 The tester.sh will reference .system/ scripts but they may not work correctly." << std::endl;
        std::cerr << "💡 Expected files:" << std::endl;
        std::cerr << "   - .system/auto_correc_main.sh (for function exercises)" << std::endl;
        std::cerr << "   - .system/auto_correc_program.sh (for program exercises)" << std::endl;
        return false;
    }

    // Check for required template files
    bool hasMain = fileExists(".system/auto_correc_main.sh");
    bool hasProgram = fileExists(".system/auto_correc_program.sh");

    if (!hasMain || !hasProgram) {
        std::cerr << "⚠️  Warning: Some .system/ template files are missing:" << std::endl;
        if (!hasMain) {
            std::cerr << "   - .system/auto_correc_main.sh (for function exercises)" << std::endl;
        }
        if (!hasProgram) {
            std::cerr << "   - .system/auto_correc_program.sh (for program exercises)" << std::endl;
        }
        std::cerr << "💡 These scripts are required for the tester.sh to work correctly." << std::endl;
        std::cerr << "💡 You can create them or copy from a 42-grademe repository." << std::endl;
        return false;
    }

    return true;
}

bool ExerciseSaver::writeToFile(const std::string &path, const std::string &content) {
    std::ofstream file(path.c_str());
    if (!file.is_open()) {
        _lastError = "Failed to open file for writing: " + path;

        // Provide helpful hints
        if (path.find("/subjects/generated/") == 0) {
            _lastError += "\n💡 Tip: Ensure the 'subjects/generated' directory exists and you have write permissions.";
        }

        // Check if parent directory exists
        size_t lastSlash = path.find_last_of('/');
        if (lastSlash != std::string::npos) {
            std::string parentDir = path.substr(0, lastSlash);
            struct stat st;
            if (stat(parentDir.c_str(), &st) != 0) {
                _lastError += "\n💡 Tip: Parent directory does not exist: " + parentDir;
            }
        }

        return false;
    }

    file << content;

    // Check if write was successful
    if (file.fail() || file.bad()) {
        _lastError = "Error writing to file: " + path;
        file.close();
        return false;
    }

    file.close();
    return true;
}

std::string ExerciseSaver::generateUniqueName(const std::string &baseName,
                                              const std::string &rank) {
    std::string exercisePath = _baseGeneratedPath + "/" + rank + "/" + baseName;

    if (!fileExists(exercisePath)) {
        return baseName;
    }

    // Append timestamp to make unique
    std::stringstream ss;
    time_t now = time(0);
    struct tm *tstruct = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", tstruct);
    ss << baseName << "_" << buffer;
    return ss.str();
}

std::string ExerciseSaver::getCurrentTimestamp() const {
    time_t now = time(0);
    struct tm *tstruct = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tstruct);
    return std::string(buffer);
}

std::string ExerciseSaver::metadataToJson(const ExerciseMetadata &metadata) const {
    std::stringstream ss;

    ss << "{\n";
    ss << "  \"exercise_name\": \"" << metadata.exercise_name << "\",\n";
    ss << "  \"rank\": \"" << metadata.rank << "\",\n";
    ss << "  \"level\": \"" << metadata.level << "\",\n";
    ss << "  \"exercise_type\": \"" << metadata.exercise_type << "\",\n";
    ss << "  \"generated_at\": \"" << metadata.generated_at << "\",\n";
    ss << "  \"llm_model\": \"" << metadata.llm_model << "\",\n";
    ss << "  \"prompt\": \"" << metadata.prompt << "\",\n";
    ss << "  \"quality_scores\": {\n";
    ss << "    \"overall\": " << metadata.overall_score << ",\n";
    ss << "    \"compilation\": " << metadata.compilation_score << ",\n";
    ss << "    \"test_coverage\": " << metadata.test_coverage_score << ",\n";
    ss << "    \"subject_clarity\": " << metadata.subject_clarity_score << ",\n";
    ss << "    \"code_quality\": " << metadata.code_quality_score << "\n";
    ss << "  },\n";
    ss << "  \"validation_rounds\": " << metadata.validation_rounds << ",\n";
    ss << "  \"approved\": " << (metadata.approved ? "true" : "false") << ",\n";

    // Rating system
    ss << "  \"ratings\": [";
    for (size_t i = 0; i < metadata.ratings.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "{\"user\": \"" << metadata.ratings[i].first
           << "\", \"score\": " << metadata.ratings[i].second << "}";
    }
    ss << "],\n";
    ss << "  \"average_rating\": " << metadata.average_rating << ",\n";
    ss << "  \"rating_count\": " << metadata.rating_count << ",\n";

    // Import tracking
    if (!metadata.imported_at.empty()) {
        ss << "  \"imported_at\": \"" << metadata.imported_at << "\",\n";
        ss << "  \"imported_by\": \"" << metadata.imported_by << "\",\n";
        ss << "  \"original_author\": \"" << metadata.original_author << "\",\n";
    }

    ss << "  \"shareable\": true\n";
    ss << "}";

    return ss.str();
}

bool ExerciseSaver::saveExercise(const ExerciseData &exercise,
                                const QualityReport &report,
                                const std::string &rank,
                                const std::string &level,
                                const std::string &prompt,
                                const std::string &compilationLog,
                                const std::string &testResults,
                                const std::string &llmModel,
                                int validationRounds) {
    // Check if required .system/ scripts exist
    checkSystemScripts();

    // Generate unique exercise name
    std::string uniqueName = generateUniqueName(exercise.exercise_name, rank);

    // Create base directory structure
    std::string basePath = _baseGeneratedPath + "/" + rank + "/" + uniqueName;
    if (!createDirectory(basePath)) {
        return false;
    }

    // Create attachment directory
    std::string attachmentPath = basePath + "/attachment";
    if (!createDirectory(attachmentPath)) {
        return false;
    }

    // Create .validation directory
    std::string validationPath = basePath + "/.validation";
    if (!createDirectory(validationPath)) {
        return false;
    }

    // Save subject.en.txt
    std::string subjectPath = attachmentPath + "/subject.en.txt";
    if (!writeToFile(subjectPath, exercise.subject)) {
        return false;
    }

    // Save solution.c (or .cpp based on content)
    std::string solutionExt = (exercise.solution_code.find("#include <iostream>") != std::string::npos ||
                               exercise.solution_code.find("std::") != std::string::npos ||
                               exercise.solution_code.find("class ") != std::string::npos) ? ".cpp" : ".c";
    std::string solutionPath = basePath + "/solution" + solutionExt;
    if (!writeToFile(solutionPath, exercise.solution_code)) {
        return false;
    }

    // Save main.c if function exercise
    if (exercise.exercise_type == "function") {
        std::string mainContent = "#include <stdio.h>\n\n";
        mainContent += "int main(int argc, char **argv) {\n";
        mainContent += "    (void)argc;\n";
        mainContent += "    return 0;\n";
        mainContent += "}\n";

        std::string mainPath = basePath + "/main.c";
        if (!writeToFile(mainPath, mainContent)) {
            return false;
        }
    }

    // Save tester.sh
    std::string testerPath = basePath + "/tester.sh";
    std::string testerContent = "#!/bin/bash\n\n";
    testerContent += "# Test harness for " + exercise.exercise_name + "\n";
    testerContent += "# Auto-generated from test cases\n\n";

    // Determine which template to use
    std::string templateFile = ".system/auto_correc_main.sh";
    if (exercise.exercise_type == "program") {
        templateFile = ".system/auto_correc_program.sh";
    }

    // Generate test cases
    for (size_t i = 0; i < exercise.test_cases.size(); ++i) {
        const TestCase &tc = exercise.test_cases[i];
        testerContent += "# Test " + to_string(i + 1) + ": " + tc.description + "\n";
        testerContent += "# Explanation: " + tc.explanation + "\n";

        std::string args;
        for (size_t j = 0; j < tc.input.size(); ++j) {
            if (j > 0) args += " ";
            args += "\"" + tc.input[j] + "\"";
        }

        testerContent += "bash " + templateFile + " $FILE $ASSIGN " + args + "\n\n";
    }

    if (!writeToFile(testerPath, testerContent)) {
        return false;
    }

    // Make tester.sh executable
    std::string chmodCmd = "chmod +x \"" + testerPath + "\"";
    system(chmodCmd.c_str());

    // Create and save metadata.json
    ExerciseMetadata metadata;
    metadata.exercise_name = exercise.exercise_name;
    metadata.rank = rank;
    metadata.level = level;
    metadata.exercise_type = exercise.exercise_type;
    metadata.generated_at = getCurrentTimestamp();
    metadata.llm_model = llmModel;
    metadata.prompt = prompt;
    metadata.overall_score = report.overall_score;
    metadata.compilation_score = report.compilation_score;
    metadata.test_coverage_score = report.test_coverage_score;
    metadata.subject_clarity_score = report.subject_clarity_score;
    metadata.code_quality_score = report.code_quality_score;
    metadata.validation_rounds = validationRounds;
    metadata.approved = report.approved;

    std::string metadataPath = basePath + "/metadata.json";
    if (!writeToFile(metadataPath, metadataToJson(metadata))) {
        return false;
    }

    // Save validation artifacts
    // Quality report
    std::string qualityReportPath = validationPath + "/quality_report.json";
    std::stringstream qualityJson;
    qualityJson << "{\n";
    qualityJson << "  \"overall_score\": " << report.overall_score << ",\n";
    qualityJson << "  \"compilation\": {\n";
    qualityJson << "    \"score\": " << report.compilation_score << ",\n";
    qualityJson << "    \"details\": \"" << report.compilation_details << "\"\n";
    qualityJson << "  },\n";
    qualityJson << "  \"test_coverage\": {\n";
    qualityJson << "    \"score\": " << report.test_coverage_score << ",\n";
    qualityJson << "    \"details\": \"" << report.test_coverage_details << "\"\n";
    qualityJson << "  },\n";
    qualityJson << "  \"subject_clarity\": {\n";
    qualityJson << "    \"score\": " << report.subject_clarity_score << ",\n";
    qualityJson << "    \"details\": \"" << report.subject_clarity_details << "\"\n";
    qualityJson << "  },\n";
    qualityJson << "  \"code_quality\": {\n";
    qualityJson << "    \"score\": " << report.code_quality_score << ",\n";
    qualityJson << "    \"details\": \"" << report.code_quality_details << "\"\n";
    qualityJson << "  },\n";
    qualityJson << "  \"approved\": " << (report.approved ? "true" : "false") << "\n";
    qualityJson << "}\n";

    if (!writeToFile(qualityReportPath, qualityJson.str())) {
        return false;
    }

    // Compilation log
    std::string compilationLogPath = validationPath + "/compilation.log";
    if (!writeToFile(compilationLogPath, compilationLog)) {
        return false;
    }

    // Test results
    std::string testResultsPath = validationPath + "/test_results.json";
    if (!writeToFile(testResultsPath, testResults)) {
        return false;
    }

    _lastSavedPath = basePath;

    std::cout << "✅ Exercise saved to: " << basePath << std::endl;
    return true;
}

const std::string &ExerciseSaver::getLastError() const {
    return _lastError;
}

std::string ExerciseSaver::getSavedExercisePath() const {
    return _lastSavedPath;
}
