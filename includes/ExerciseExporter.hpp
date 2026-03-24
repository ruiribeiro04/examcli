#ifndef EXERCISEEXPORTER_HPP
#define EXERCISEEXPORTER_HPP

#include <string>

class ExerciseExporter {
private:
    std::string _lastError;

    bool findExercisePath(const std::string &exerciseName, std::string &path);

public:
    ExerciseExporter();
    ExerciseExporter(const ExerciseExporter &other);
    ExerciseExporter &operator=(const ExerciseExporter &other);
    ~ExerciseExporter();

    bool exportExercise(const std::string &exerciseName,
                       const std::string &outputPath);
    bool exportMetadataOnly(const std::string &exerciseName,
                           const std::string &outputPath);
    const std::string &getLastError() const;
};

#endif
