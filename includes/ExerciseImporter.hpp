#ifndef EXERCISEIMPORTER_HPP
#define EXERCISEIMPORTER_HPP

#include <string>

class ExerciseImporter {
private:
    std::string _lastError;

public:
    ExerciseImporter();
    ExerciseImporter(const ExerciseImporter &other);
    ExerciseImporter &operator=(const ExerciseImporter &other);
    ~ExerciseImporter();

    bool importExercise(const std::string &archivePath,
                       const std::string &rankOverride = "");
    const std::string &getLastError() const;
};

#endif
