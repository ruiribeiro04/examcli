#include "QualityScorer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

QualityScorer::QualityScorer() : _lastError() {
}

QualityScorer::QualityScorer(const QualityScorer &other)
    : _lastError(other._lastError) {
}

QualityScorer &QualityScorer::operator=(const QualityScorer &other) {
    if (this != &other) {
        _lastError = other._lastError;
    }
    return *this;
}

QualityScorer::~QualityScorer() {
}

bool QualityScorer::calculateQuality(const ExerciseData &exercise,
                                    const std::string &compilationLog,
                                    QualityReport &report) {
    // Score each category
    report.compilation_score = scoreCompilation(compilationLog);
    report.test_coverage_score = scoreTestCoverage(exercise.test_cases);
    report.subject_clarity_score = scoreSubjectClarity(exercise.subject);
    report.code_quality_score = scoreCodeQuality(exercise.solution_code);

    // Calculate weighted overall score
    report.overall_score =
        (report.compilation_score * 25) / 100 +
        (report.test_coverage_score * 30) / 100 +
        (report.subject_clarity_score * 25) / 100 +
        (report.code_quality_score * 20) / 100;

    report.approved = (report.overall_score >= 70);

    // Generate details
    std::stringstream compDetails;
    compDetails << "Compilation Score: " << report.compilation_score << "/100";
    if (report.compilation_score == 100) {
        compDetails << " (Clean compilation)";
    } else if (report.compilation_score >= 70) {
        compDetails << " (Compiled with warnings)";
    } else {
        compDetails << " (Compilation failed)";
    }
    report.compilation_details = compDetails.str();

    std::stringstream testDetails;
    testDetails << "Test Coverage: " << report.test_coverage_score << "/100 "
                << "(" << exercise.test_cases.size() << " test cases)";
    report.test_coverage_details = testDetails.str();

    std::stringstream subjectDetails;
    subjectDetails << "Subject Clarity: " << report.subject_clarity_score << "/100 "
                   << "(" << exercise.subject.length() << " characters)";
    report.subject_clarity_details = subjectDetails.str();

    std::stringstream codeDetails;
    codeDetails << "Code Quality: " << report.code_quality_score << "/100";
    report.code_quality_details = codeDetails.str();

    return true;
}

const std::string &QualityScorer::getLastError() const {
    return _lastError;
}

int QualityScorer::scoreCompilation(const std::string &compilationLog) {
    // 100 points for clean compilation, 70 for warnings, 0 for errors
    if (compilationLog.empty() || compilationLog.find("error") == std::string::npos) {
        return 100; // Clean compilation
    } else if (compilationLog.find("warning") != std::string::npos) {
        return 70; // Compiled with warnings
    } else {
        return 0; // Compilation failed
    }
}

int QualityScorer::scoreTestCoverage(const std::vector<TestCase> &testCases) {
    if (testCases.empty()) {
        return 0;
    }

    int score = 0;
    int normalCount = 0;
    int edgeCount = 0;
    int errorCount = 0;

    // Categorize test cases
    for (size_t i = 0; i < testCases.size(); ++i) {
        const TestCase &tc = testCases[i];
        std::string desc = tc.description;
        std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);

        if (desc.find("edge") != std::string::npos ||
            desc.find("boundary") != std::string::npos ||
            desc.find("limit") != std::string::npos) {
            edgeCount++;
        } else if (desc.find("error") != std::string::npos ||
                   desc.find("invalid") != std::string::npos ||
                   desc.find("null") != std::string::npos) {
            errorCount++;
        } else {
            normalCount++;
        }
    }

    // Score: normal 30pts, edge 40pts, error 30pts
    // Calculate based on minimum required (2 normal, 2 edge, 1 error)
    if (normalCount >= 2) score += 30;
    else if (normalCount >= 1) score += 15;

    if (edgeCount >= 2) score += 40;
    else if (edgeCount >= 1) score += 20;

    if (errorCount >= 1) score += 30;
    else if (errorCount >= 1) score += 15;

    // Bonus points for additional test cases
    int totalTests = testCases.size();
    if (totalTests > 5) {
        score += std::min(10, totalTests - 5);
    }

    return std::min(100, score);
}

int QualityScorer::scoreSubjectClarity(const std::string &subject) {
    int score = 0;

    // Length score (20 points)
    size_t length = subject.length();
    if (length >= 500) score += 20;
    else if (length >= 300) score += 15;
    else if (length >= 150) score += 10;
    else if (length >= 50) score += 5;

    // Examples score (20 points)
    if (subject.find("Example") != std::string::npos ||
        subject.find("example") != std::string::npos) {
        score += 20;
    } else if (subject.find("ex:") != std::string::npos ||
               subject.find("e.g.") != std::string::npos) {
        score += 10;
    }

    // Requirements score (30 points)
    int requirementsCount = 0;
    std::string lowerSubject = subject;
    std::transform(lowerSubject.begin(), lowerSubject.end(), lowerSubject.begin(), ::tolower);

    if (lowerSubject.find("must") != std::string::npos) requirementsCount++;
    if (lowerSubject.find("should") != std::string::npos) requirementsCount++;
    if (lowerSubject.find("require") != std::string::npos) requirementsCount++;
    if (lowerSubject.find("return") != std::string::npos) requirementsCount++;
    if (lowerSubject.find("parameter") != std::string::npos ||
        lowerSubject.find("argument") != std::string::npos) requirementsCount++;

    score += std::min(30, requirementsCount * 6);

    // Completeness score (30 points)
    if (subject.find("42") != std::string::npos || subject.find("norm") != std::string::npos) {
        score += 10;
    }
    if (subject.find("function") != std::string::npos || subject.find("program") != std::string::npos) {
        score += 10;
    }
    if (subject.find("prototype") != std::string::npos || subject.find("int ") != std::string::npos) {
        score += 10;
    }

    return std::min(100, score);
}

int QualityScorer::scoreCodeQuality(const std::string &code) {
    int score = 0;
    std::string lowerCode = code;
    std::transform(lowerCode.begin(), lowerCode.end(), lowerCode.begin(), ::tolower);

    // 42 norms adherence (30 points)
    if (code.find("/*") != std::string::npos && code.find("*/") != std::string::npos) {
        score += 10; // Has comments
    }
    if (code.find("#include") != std::string::npos) {
        score += 10; // Has includes
    }
    if (code.find("int ") != std::string::npos || code.find("char ") != std::string::npos ||
        code.find("void ") != std::string::npos) {
        score += 10; // Has proper types
    }

    // Memory management (30 points)
    if (code.find("malloc") != std::string::npos || code.find("free") != std::string::npos) {
        if (code.find("free") != std::string::npos) {
            score += 15; // Properly frees memory
        }
        if (code.find("malloc") != std::string::npos) {
            score += 15; // Uses dynamic allocation
        }
    } else {
        score += 30; // No dynamic memory needed (good)
    }

    // Error handling (20 points)
    if (lowerCode.find("if") != std::string::npos) {
        score += 10; // Has conditionals
    }
    if (lowerCode.find("return") != std::string::npos) {
        score += 10; // Has returns
    }

    // Organization (20 points)
    int lineCount = std::count(code.begin(), code.end(), '\n');
    if (lineCount > 10 && lineCount < 100) {
        score += 10; // Reasonable length
    }
    if (code.find("{") != std::string::npos && code.find("}") != std::string::npos) {
        score += 10; // Has code blocks
    }

    return std::min(100, score);
}
