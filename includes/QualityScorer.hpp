#ifndef QUALITYSCORER_HPP
#define QUALITYSCORER_HPP

#include <string>
#include <map>
#include "Generator.hpp"

struct QualityReport {
    int overall_score;
    int compilation_score;
    int test_coverage_score;
    int subject_clarity_score;
    int code_quality_score;
    std::string compilation_details;
    std::string test_coverage_details;
    std::string subject_clarity_details;
    std::string code_quality_details;
    bool approved;

    QualityReport() : overall_score(0), compilation_score(0),
                      test_coverage_score(0), subject_clarity_score(0),
                      code_quality_score(0), compilation_details(),
                      test_coverage_details(), subject_clarity_details(),
                      code_quality_details(), approved(false) {}
};

class QualityScorer {
private:
    std::string _lastError;

    int scoreCompilation(const std::string &compilationLog);
    int scoreTestCoverage(const std::vector<TestCase> &testCases);
    int scoreSubjectClarity(const std::string &subject);
    int scoreCodeQuality(const std::string &code);

public:
    QualityScorer();
    QualityScorer(const QualityScorer &other);
    QualityScorer &operator=(const QualityScorer &other);
    ~QualityScorer();

    bool calculateQuality(const ExerciseData &exercise,
                         const std::string &compilationLog,
                         QualityReport &report);
    const std::string &getLastError() const;
};

#endif
