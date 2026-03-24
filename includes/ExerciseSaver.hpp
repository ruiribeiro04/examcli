#ifndef EXERCISESAVER_HPP
#define EXERCISESAVER_HPP

#include <string>
#include "Generator.hpp"
#include "QualityScorer.hpp"

struct ExerciseMetadata {
    std::string exercise_name;
    std::string rank;
    std::string level;
    std::string exercise_type;
    std::string generated_at;
    std::string llm_model;
    std::string prompt;
    int overall_score;
    int compilation_score;
    int test_coverage_score;
    int subject_clarity_score;
    int code_quality_score;
    int validation_rounds;
    bool approved;

    // Rating system fields
    std::vector<std::pair<std::string, int> > ratings; // user -> score
    double average_rating;
    int rating_count;

    // Import tracking
    std::string imported_at;
    std::string imported_by;
    std::string original_author;

    ExerciseMetadata() : rank("rank02"), level("level0"), exercise_type("function"),
                         generated_at(), llm_model(), prompt(), overall_score(0),
                         compilation_score(0), test_coverage_score(0),
                         subject_clarity_score(0), code_quality_score(0),
                         validation_rounds(0), approved(false),
                         average_rating(0.0), rating_count(0),
                         imported_at(), imported_by(), original_author() {}
};

class ExerciseSaver {
private:
    std::string _lastError;
    std::string _baseGeneratedPath;

    bool createDirectory(const std::string &path);
    bool fileExists(const std::string &path) const;
    bool writeToFile(const std::string &path, const std::string &content);
    bool checkSystemScripts() const;
    std::string generateUniqueName(const std::string &baseName, const std::string &rank);
    std::string getCurrentTimestamp() const;
    std::string metadataToJson(const ExerciseMetadata &metadata) const;

public:
    ExerciseSaver();
    ExerciseSaver(const ExerciseSaver &other);
    ExerciseSaver &operator=(const ExerciseSaver &other);
    ~ExerciseSaver();

    bool saveExercise(const ExerciseData &exercise,
                     const QualityReport &report,
                     const std::string &rank,
                     const std::string &level,
                     const std::string &prompt,
                     const std::string &compilationLog,
                     const std::string &testResults,
                     const std::string &llmModel = "",
                     int validationRounds = 0);
    const std::string &getLastError() const;
    std::string getSavedExercisePath() const;

private:
    std::string _lastSavedPath;
};

#endif
