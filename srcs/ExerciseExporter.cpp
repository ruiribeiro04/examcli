#include "ExerciseExporter.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>

ExerciseExporter::ExerciseExporter() : _lastError() {
}

ExerciseExporter::ExerciseExporter(const ExerciseExporter &other)
    : _lastError(other._lastError) {
}

ExerciseExporter &ExerciseExporter::operator=(const ExerciseExporter &other) {
    if (this != &other) {
        _lastError = other._lastError;
    }
    return *this;
}

ExerciseExporter::~ExerciseExporter() {
}

bool ExerciseExporter::findExercisePath(const std::string &exerciseName, std::string &path) {
    std::string exerciseBasePath = "subjects/generated";

    // Try to find in rank02, rank03, rank04, rank05
    const char *ranks[] = {"rank02", "rank03", "rank04", "rank05"};
    for (size_t i = 0; i < 4; ++i) {
        std::string testPath = exerciseBasePath + "/" + ranks[i] + "/" + exerciseName;
        struct stat st;
        if (stat(testPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            path = testPath;
            return true;
        }
    }

    return false;
}

bool ExerciseExporter::exportExercise(const std::string &exerciseName,
                                     const std::string &outputPath) {
    std::string fullExercisePath;
    if (!findExercisePath(exerciseName, fullExercisePath)) {
        _lastError = "Exercise not found: " + exerciseName;
        return false;
    }

    // Validate exercise structure
    std::string metadataPath = fullExercisePath + "/metadata.json";
    std::string subjectPath = fullExercisePath + "/attachment/subject.en.txt";
    std::string solutionPath = fullExercisePath + "/solution.c";
    std::string testerPath = fullExercisePath + "/tester.sh";

    struct stat st;
    if (stat(metadataPath.c_str(), &st) != 0) {
        _lastError = "Invalid exercise: missing metadata.json";
        return false;
    }
    if (stat(subjectPath.c_str(), &st) != 0) {
        _lastError = "Invalid exercise: missing attachment/subject.en.txt";
        return false;
    }
    if (stat(solutionPath.c_str(), &st) != 0 && stat((fullExercisePath + "/solution.cpp").c_str(), &st) != 0) {
        _lastError = "Invalid exercise: missing solution.c or solution.cpp";
        return false;
    }
    if (stat(testerPath.c_str(), &st) != 0) {
        _lastError = "Invalid exercise: missing tester.sh";
        return false;
    }

    // Create tar.gz archive
    std::string archiveName = exerciseName + ".tar.gz";
    std::string fullOutputPath = outputPath;

    // If outputPath is a directory, append filename
    if (outputPath.empty() || outputPath[outputPath.length() - 1] == '/') {
        fullOutputPath += archiveName;
    }

    // Use tar command to create archive
    std::string tarCommand = "tar -czf \"" + fullOutputPath + "\" -C \"" +
                            fullExercisePath + "/..\" \"" + exerciseName + "\"";

    std::cout << "Creating archive: " << fullOutputPath << std::endl;
    int result = system(tarCommand.c_str());

    if (result != 0) {
        _lastError = "Failed to create tar.gz archive";
        return false;
    }

    std::cout << "✅ Exercise exported to: " << fullOutputPath << std::endl;
    return true;
}

bool ExerciseExporter::exportMetadataOnly(const std::string &exerciseName,
                                         const std::string &outputPath) {
    std::string fullExercisePath;
    if (!findExercisePath(exerciseName, fullExercisePath)) {
        _lastError = "Exercise not found: " + exerciseName;
        return false;
    }

    // Validate metadata exists
    std::string metadataPath = fullExercisePath + "/metadata.json";
    struct stat st;
    if (stat(metadataPath.c_str(), &st) != 0) {
        _lastError = "Invalid exercise: missing metadata.json";
        return false;
    }

    // Create output filename
    std::string archiveName = exerciseName + "_metadata.json";
    std::string fullOutputPath = outputPath;

    // If outputPath is a directory, append filename
    if (outputPath.empty() || outputPath[outputPath.length() - 1] == '/') {
        fullOutputPath += archiveName;
    }

    // Copy metadata file to output
    std::ifstream src(metadataPath.c_str(), std::ios::binary);
    std::ofstream dst(fullOutputPath.c_str(), std::ios::binary);

    if (!src.is_open() || !dst.is_open()) {
        _lastError = "Failed to copy metadata file";
        return false;
    }

    dst << src.rdbuf();
    src.close();
    dst.close();

    std::cout << "✅ Metadata exported to: " << fullOutputPath << std::endl;
    return true;
}

const std::string &ExerciseExporter::getLastError() const {
    return _lastError;
}
