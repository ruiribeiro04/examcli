#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <string>
#include <vector>
#include <csignal>
#include "Generator.hpp"

enum ValidationStage {
    STAGE_LLM_GENERATION,
    STAGE_SYNTAX_VALIDATION,
    STAGE_COMPILATION,
    STAGE_EXECUTION_TESTING,
    STAGE_TEST_GENERATION,
    STAGE_FINAL_VALIDATION
};

struct ValidationResult {
    bool success;
    ValidationStage stage;
    std::string message;
    int retries;
    bool cancelled;

    ValidationResult() : success(false), stage(STAGE_LLM_GENERATION),
                        message(), retries(0), cancelled(false) {}
};

class Validator {
private:
    std::string _lastError;
    std::string _tempDir;
    std::string _compilationLog;
    std::string _testResults;
    std::string _cacheDir;
    static volatile sig_atomic_t _cancelRequested;

    static void signalHandler(int signal);
    bool validateSyntax(const ExerciseData &exercise);
    bool validateCompilation(const ExerciseData &exercise);
    bool validateExecution(const ExerciseData &exercise);
    bool validateTestGeneration(const ExerciseData &exercise);
    bool validateFinal(const ExerciseData &exercise);
    std::string runCommand(const std::string &cmd) const;
    bool fileExists(const std::string &path) const;
    bool checkCancellation() const;
    std::string computeHash(const std::string &str) const;
    bool getCachedCompilation(const std::string &hash);
    void cacheCompilation(const std::string &hash, const std::string &log);

    // Parallel test execution (task 12.5)
    bool validateExecutionParallel(const ExerciseData &exercise);

public:
    Validator();
    Validator(const Validator &other);
    Validator &operator=(const Validator &other);
    ~Validator();

    bool validate(ExerciseData &exercise, ValidationResult &result);
    const std::string &getLastError() const;
    const std::string &getCompilationLog() const;
    const std::string &getTestResults() const;
    static void setupSignalHandler();
    static bool isCancelled();
};

#endif
