#include "ExerciseImporter.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>
#include <cstring>

// C++98 compatible to_string helper
namespace {
    template<typename T>
    std::string to_string(T value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
}

ExerciseImporter::ExerciseImporter() : _lastError() {
}

ExerciseImporter::ExerciseImporter(const ExerciseImporter &other)
    : _lastError(other._lastError) {
}

ExerciseImporter &ExerciseImporter::operator=(const ExerciseImporter &other) {
    if (this != &other) {
        _lastError = other._lastError;
    }
    return *this;
}

ExerciseImporter::~ExerciseImporter() {
}

bool ExerciseImporter::importExercise(const std::string &archivePath,
                                     const std::string &rankOverride) {
    // Check if archive exists
    struct stat st;
    if (stat(archivePath.c_str(), &st) != 0) {
        _lastError = "Archive not found: " + archivePath;
        return false;
    }

    // Check if it's a .tar.gz file
    if (archivePath.find(".tar.gz") == std::string::npos &&
        archivePath.find(".tgz") == std::string::npos) {
        _lastError = "Invalid archive format (must be .tar.gz or .tgz)";
        return false;
    }

    // Extract to temporary directory
    std::string tempDir = "/tmp/examcli_import";
    std::string mkdirCmd = "mkdir -p \"" + tempDir + "\"";
    system(mkdirCmd.c_str());

    std::string extractCmd = "tar -xzf \"" + archivePath + "\" -C \"" + tempDir + "\"";
    std::cout << "Extracting archive..." << std::endl;
    int result = system(extractCmd.c_str());

    if (result != 0) {
        _lastError = "Failed to extract archive";
        return false;
    }

    // Find the extracted exercise directory
    // Assuming the archive contains a single directory with the exercise name
    std::string exerciseDir;
    std::string findCmd = "ls \"" + tempDir + "\"";
    FILE *pipe = popen(findCmd.c_str(), "r");
    if (!pipe) {
        _lastError = "Failed to read extracted directory";
        return false;
    }

    char buffer[128];
    if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        exerciseDir = buffer;
        // Remove trailing newline
        if (!exerciseDir.empty() && exerciseDir[exerciseDir.length() - 1] == '\n') {
            exerciseDir.erase(exerciseDir.length() - 1);
        }
    }
    pclose(pipe);

    if (exerciseDir.empty()) {
        _lastError = "No exercise directory found in archive";
        return false;
    }

    std::string extractedPath = tempDir + "/" + exerciseDir;

    // Validate exercise structure
    std::string metadataPath = extractedPath + "/metadata.json";
    std::string subjectPath = extractedPath + "/attachment/subject.en.txt";
    std::string solutionPath = extractedPath + "/solution.c";
    std::string testerPath = extractedPath + "/tester.sh";

    if (stat(metadataPath.c_str(), &st) != 0) {
        _lastError = "Invalid exercise: missing metadata.json";
        return false;
    }
    if (stat(subjectPath.c_str(), &st) != 0) {
        _lastError = "Invalid exercise: missing attachment/subject.en.txt";
        return false;
    }
    if (stat(solutionPath.c_str(), &st) != 0 && stat((extractedPath + "/solution.cpp").c_str(), &st) != 0) {
        _lastError = "Invalid exercise: missing solution.c or solution.cpp";
        return false;
    }
    if (stat(testerPath.c_str(), &st) != 0) {
        _lastError = "Invalid exercise: missing tester.sh";
        return false;
    }

    // Determine target rank
    std::string targetRank = rankOverride;
    if (targetRank.empty()) {
        // Try to read rank from metadata.json
        std::ifstream metaFile(metadataPath.c_str());
        if (metaFile.is_open()) {
            std::string line;
            while (std::getline(metaFile, line)) {
                if (line.find("\"rank\"") != std::string::npos) {
                    size_t pos = line.find(":");
                    if (pos != std::string::npos) {
                        std::string rankValue = line.substr(pos + 1);
                        // Remove quotes, whitespace, and comma
                        size_t start = rankValue.find("\"");
                        size_t end = rankValue.find("\"", start + 1);
                        if (start != std::string::npos && end != std::string::npos) {
                            targetRank = rankValue.substr(start + 1, end - start - 1);
                        }
                        break;
                    }
                }
            }
            metaFile.close();
        }
    }

    if (targetRank.empty()) {
        targetRank = "rank02"; // Default
    }

    // Check if exercise already exists and generate unique name if needed
    std::string baseExerciseName = exerciseDir;
    std::string targetPath = "subjects/generated/" + targetRank + "/" + baseExerciseName;

    if (stat(targetPath.c_str(), &st) == 0) {
        // Exercise exists, append timestamp
        time_t now = time(0);
        struct tm *tstruct = localtime(&now);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", tstruct);
        baseExerciseName = exerciseDir + "_" + buffer;
        targetPath = "subjects/generated/" + targetRank + "/" + baseExerciseName;
    }

    // Move exercise to subjects/generated/
    std::string targetDir = "subjects/generated/" + targetRank;
    std::string createDirCmd = "mkdir -p \"" + targetDir + "\"";
    system(createDirCmd.c_str());

    std::string moveCmd = "mv \"" + extractedPath + "\" \"" + targetPath + "\"";
    result = system(moveCmd.c_str());

    if (result != 0) {
        _lastError = "Failed to move exercise to subjects/generated/";
        return false;
    }

    // Update metadata.json to track import
    std::string updatedMetadataPath = targetPath + "/metadata.json";
    // Read existing metadata
    std::ifstream inFile(updatedMetadataPath.c_str());
    std::stringstream metadataBuffer;
    metadataBuffer << inFile.rdbuf();
    inFile.close();

    std::string metadataContent = metadataBuffer.str();

    // Add import tracking fields if not present
    if (metadataContent.find("imported_at") == std::string::npos) {
        // Insert import tracking before closing brace
        size_t lastBrace = metadataContent.find_last_of("}");
        if (lastBrace != std::string::npos) {
            time_t now = time(0);
            struct tm *tstruct = localtime(&now);
            char timeBuffer[80];
            strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", tstruct);

            std::string importInfo = ",\n  \"imported_at\": \"" + std::string(timeBuffer) + "\"";
            importInfo += ",\n  \"imported_by\": \"user\"";
            importInfo += ",\n  \"original_author\": \"unknown\"";

            metadataContent.insert(lastBrace, importInfo);

            // Write updated metadata
            std::ofstream outFile(updatedMetadataPath.c_str());
            outFile << metadataContent;
            outFile.close();
        }
    }

    // Clean up temp directory
    std::string cleanupCmd = "rm -rf \"" + tempDir + "\"";
    system(cleanupCmd.c_str());

    std::cout << "✅ Exercise imported to: " << targetPath << std::endl;
    return true;
}

const std::string &ExerciseImporter::getLastError() const {
    return _lastError;
}
