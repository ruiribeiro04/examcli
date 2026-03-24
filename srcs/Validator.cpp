#include "Validator.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>

// Static member initialization
volatile sig_atomic_t Validator::_cancelRequested = 0;

void Validator::signalHandler(int signal) {
    if (signal == SIGINT) {
        _cancelRequested = 1;
        std::cout << "\n⚠️  Cancel requested. Finishing current validation stage...\n" << std::flush;
    }
}

void Validator::setupSignalHandler() {
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, NULL);
}

bool Validator::isCancelled() {
    return _cancelRequested != 0;
}

bool Validator::checkCancellation() const {
    return _cancelRequested != 0;
}

Validator::Validator() : _lastError(), _tempDir("/tmp/examcli_validator"),
    _compilationLog(), _testResults(), _cacheDir("/tmp/examcli_compile_cache") {
    // Create cache directory
    mkdir(_cacheDir.c_str(), 0755);
}

// Simple hash function for caching (djb2 algorithm)
std::string Validator::computeHash(const std::string &str) const {
    unsigned long hash = 5381;
    for (size_t i = 0; i < str.length(); ++i) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(str[i]); // hash * 33 + c
    }

    std::stringstream ss;
    ss << std::hex << hash;
    return ss.str();
}

bool Validator::getCachedCompilation(const std::string &hash) {
    std::string cacheFile = _cacheDir + "/" + hash;
    if (!fileExists(cacheFile)) {
        return false;
    }

    std::ifstream file(cacheFile.c_str());
    if (!file.is_open()) {
        return false;
    }

    std::getline(file, _compilationLog);
    file.close();

    // If log is empty, compilation was successful
    return true;
}

void Validator::cacheCompilation(const std::string &hash, const std::string &log) {
    std::string cacheFile = _cacheDir + "/" + hash;
    std::ofstream file(cacheFile.c_str());
    if (file.is_open()) {
        file << log;
        file.close();
    }
}

Validator::Validator(const Validator &other)
    : _lastError(other._lastError), _tempDir(other._tempDir),
      _compilationLog(other._compilationLog), _testResults(other._testResults),
      _cacheDir(other._cacheDir) {
}

Validator &Validator::operator=(const Validator &other) {
    if (this != &other) {
        _lastError = other._lastError;
        _tempDir = other._tempDir;
        _compilationLog = other._compilationLog;
        _testResults = other._testResults;
        _cacheDir = other._cacheDir;
    }
    return *this;
}

Validator::~Validator() {
}

bool Validator::validate(ExerciseData &exercise, ValidationResult &result) {
    result.success = false;
    result.retries = 0;
    result.cancelled = false;

    // Fail-fast: Check .system/ scripts exist before starting validation
    std::cout << "🔍 Checking system dependencies..." << std::endl;
    std::string systemScript = (exercise.exercise_type == "function") ?
        ".system/auto_correc_main.sh" : ".system/auto_correc_program.sh";

    if (!fileExists(systemScript)) {
        _lastError = "Required 42-grademe script not found: " + systemScript +
                    "\nPlease ensure the .system/ directory is properly configured.";
        result.stage = STAGE_LLM_GENERATION;
        result.message = "System configuration error: " + _lastError;
        std::cerr << "❌ " << _lastError << "\n";
        return false;
    }
    std::cout << "✓ System dependencies found\n" << std::endl;

    // Fail-fast: Check temp directory is writable
    std::string testFile = _tempDir + "/test_write";
    std::ofstream test(testFile.c_str());
    if (!test.is_open()) {
        _lastError = "Cannot write to temp directory: " + _tempDir +
                    "\nCheck permissions and available disk space.";
        result.stage = STAGE_LLM_GENERATION;
        result.message = "Filesystem error: " + _lastError;
        std::cerr << "❌ " << _lastError << "\n";
        return false;
    }
    test.close();
    remove(testFile.c_str());

    // Stage 1: Syntax Validation
    std::cout << "📋 Stage 1: Validating syntax..." << std::endl;
    result.stage = STAGE_SYNTAX_VALIDATION;
    if (!validateSyntax(exercise)) {
        result.message = "Syntax validation failed: " + _lastError;
        return false;
    }
    std::cout << "✓ Syntax validation passed" << std::endl;

    if (checkCancellation()) {
        result.cancelled = true;
        result.message = "Validation cancelled by user";
        std::cout << "\n⚠️  Validation cancelled" << std::endl;
        return false;
    }

    // Stage 2: Compilation
    std::cout << "🔨 Stage 2: Compiling solution..." << std::endl;
    result.stage = STAGE_COMPILATION;
    if (!validateCompilation(exercise)) {
        result.message = "Compilation failed: " + _lastError;
        return false;
    }
    std::cout << "✓ Compilation successful" << std::endl;

    if (checkCancellation()) {
        result.cancelled = true;
        result.message = "Validation cancelled by user";
        std::cout << "\n⚠️  Validation cancelled" << std::endl;
        return false;
    }

    // Stage 3: Execution Testing
    std::cout << "⚙️  Stage 3: Testing execution..." << std::endl;
    result.stage = STAGE_EXECUTION_TESTING;
    if (!validateExecution(exercise)) {
        result.message = "Execution testing failed: " + _lastError;
        return false;
    }
    std::cout << "✓ Execution testing passed" << std::endl;

    if (checkCancellation()) {
        result.cancelled = true;
        result.message = "Validation cancelled by user";
        std::cout << "\n⚠️  Validation cancelled" << std::endl;
        return false;
    }

    // Stage 4: Test Generation
    std::cout << "📝 Stage 4: Generating test harness..." << std::endl;
    result.stage = STAGE_TEST_GENERATION;
    if (!validateTestGeneration(exercise)) {
        result.message = "Test generation failed: " + _lastError;
        return false;
    }
    std::cout << "✓ Test harness generated" << std::endl;

    if (checkCancellation()) {
        result.cancelled = true;
        result.message = "Validation cancelled by user";
        std::cout << "\n⚠️  Validation cancelled" << std::endl;
        return false;
    }

    // Stage 5: Final Validation
    std::cout << "🔍 Stage 5: Final validation..." << std::endl;
    result.stage = STAGE_FINAL_VALIDATION;
    if (!validateFinal(exercise)) {
        result.message = "Final validation failed: " + _lastError;
        return false;
    }
    std::cout << "✓ Final validation passed" << std::endl;

    result.success = true;
    result.message = "All validation stages passed";
    return true;
}

const std::string &Validator::getLastError() const {
    return _lastError;
}

const std::string &Validator::getCompilationLog() const {
    return _compilationLog;
}

const std::string &Validator::getTestResults() const {
    return _testResults;
}

bool Validator::validateSyntax(const ExerciseData &exercise) {
    // Check if all required fields are present and valid
    if (exercise.exercise_name.empty()) {
        _lastError = "Missing exercise_name";
        return false;
    }

    if (exercise.exercise_type != "function" && exercise.exercise_type != "program") {
        _lastError = "Invalid exercise_type: must be 'function' or 'program'";
        return false;
    }

    if (exercise.subject.empty() || exercise.subject.length() < 50) {
        _lastError = "Subject too short or missing (minimum 50 characters)";
        return false;
    }

    if (exercise.solution_code.empty()) {
        _lastError = "Missing solution_code";
        return false;
    }

    if (exercise.test_cases.empty()) {
        _lastError = "No test cases provided";
        return false;
    }

    // Check test cases have required fields
    for (size_t i = 0; i < exercise.test_cases.size(); ++i) {
        const TestCase &tc = exercise.test_cases[i];
        if (tc.description.empty() || tc.expected.empty()) {
            _lastError = "Test case " + std::string(1, ('0' + i)) + " missing description or expected output";
            return false;
        }
    }

    return true;
}

bool Validator::validateCompilation(const ExerciseData &exercise) {
    // Create temp directory
    std::string mkdirCmd = "mkdir -p " + _tempDir;
    system(mkdirCmd.c_str());

    // Check cache first
    std::string codeHash = computeHash(exercise.solution_code);
    if (getCachedCompilation(codeHash)) {
        std::cout << "   Using cached compilation result..." << std::endl;

        // If cached log is empty, compilation was successful
        if (_compilationLog.empty()) {
            // Still need to create the executable for testing
            std::string solutionPath = _tempDir + "/solution.c";
            std::ofstream solFile(solutionPath.c_str());
            if (!solFile.is_open()) {
                _lastError = "Failed to create temp solution file";
                return false;
            }
            solFile << exercise.solution_code;
            solFile.close();

            std::stringstream compileCmd;
            compileCmd << "gcc -std=c99 -Wall -Wextra -Werror " << solutionPath
                      << " -o " << _tempDir << "/solution 2>&1";
            runCommand(compileCmd.str());
            return true;
        }

        // Cached compilation failed
        return false;
    }

    // Write solution to temp file
    std::string solutionPath = _tempDir + "/solution.c";
    std::ofstream solFile(solutionPath.c_str());
    if (!solFile.is_open()) {
        _lastError = "Failed to create temp solution file";
        return false;
    }

    solFile << exercise.solution_code;
    solFile.close();

    // Compile with 42 flags
    std::cout << "   Compiling with gcc -std=c99 -Wall -Wextra -Werror..." << std::flush;
    std::stringstream compileCmd;
    compileCmd << "gcc -std=c99 -Wall -Wextra -Werror " << solutionPath
               << " -o " << _tempDir << "/solution 2>&1";

    _compilationLog = runCommand(compileCmd.str());

    // Cache the result
    cacheCompilation(codeHash, _compilationLog);

    // Check if compilation succeeded
    if (!_compilationLog.empty()) {
        std::cout << " ❌" << std::endl;
        _lastError = "Compilation errors/warnings:\n" + _compilationLog;
        return false;
    }

    // Check if executable was created
    if (!fileExists(_tempDir + "/solution")) {
        std::cout << " ❌" << std::endl;
        _lastError = "Compilation failed: executable not created";
        return false;
    }

    std::cout << " ✓" << std::endl;
    return true;
}

bool Validator::validateExecution(const ExerciseData &exercise) {
    std::string executable = _tempDir + "/solution";

    if (!fileExists(executable)) {
        _lastError = "Solution executable not found";
        return false;
    }

    // Run each test case
    std::stringstream testResultsJson;
    testResultsJson << "[\n";

    for (size_t i = 0; i < exercise.test_cases.size(); ++i) {
        const TestCase &tc = exercise.test_cases[i];

        // Build command with arguments
        std::stringstream cmd;
        cmd << executable;
        for (size_t j = 0; j < tc.input.size(); ++j) {
            cmd << " \"" << tc.input[j] << "\"";
        }

        // Run with timeout (5 seconds)
        std::stringstream timeoutCmd;
        timeoutCmd << "timeout 5s " << cmd.str() << " 2>&1";

        std::string output = runCommand(timeoutCmd.str());

        // Compare output
        bool passed = (output == tc.expected || output == tc.expected + "\n");

        testResultsJson << "  {\n"
                       << "    \"test_case\": " << (i + 1) << ",\n"
                       << "    \"description\": \"" << tc.description << "\",\n"
                       << "    \"passed\": " << (passed ? "true" : "false") << ",\n"
                       << "    \"expected\": \"" << tc.expected << "\",\n"
                       << "    \"actual\": \"" << output << "\"\n"
                       << "  }" << (i < exercise.test_cases.size() - 1 ? "," : "") << "\n";

        if (!passed) {
            _lastError = "Test case " + std::string(1, ('0' + i + 1)) + " failed: expected '" +
                        tc.expected + "', got '" + output + "'";
            _testResults = testResultsJson.str() + "]";
            return false;
        }
    }

    testResultsJson << "]";
    _testResults = testResultsJson.str();

    return true;
}

// Parallel test execution (task 12.5)
// Runs independent test cases in parallel using background processes
bool Validator::validateExecutionParallel(const ExerciseData &exercise) {
    std::string executable = _tempDir + "/solution";

    if (!fileExists(executable)) {
        _lastError = "Solution executable not found";
        return false;
    }

    // For C++98 without <thread>, we use shell background jobs
    std::stringstream testResultsJson;
    testResultsJson << "[\n";

    // Create temporary directory for test outputs
    std::string testOutputDir = _tempDir + "/test_outputs";
    std::string mkdirCmd = "mkdir -p " + testOutputDir;
    system(mkdirCmd.c_str());

    // Launch all tests in parallel using background processes
    for (size_t i = 0; i < exercise.test_cases.size(); ++i) {
        const TestCase &tc = exercise.test_cases[i];

        // Build command with arguments
        std::stringstream cmd;
        cmd << executable;
        for (size_t j = 0; j < tc.input.size(); ++j) {
            cmd << " \"" << tc.input[j] << "\"";
        }

        // Run with timeout in background, redirect output to file
        std::stringstream outputFile;
        outputFile << testOutputDir << "/test_" << i << ".txt";

        std::stringstream runCmd;
        runCmd << "(timeout 5s " << cmd.str() << " 2>&1 > " << outputFile.str()
               << "; echo $? > " << testOutputDir << "/test_" << i << ".exit) &";

        // Execute in background
        system(runCmd.str().c_str());
    }

    // Wait for all background processes to complete
    sleep(1);  // Give processes time to start
    system("wait 2>/dev/null");  // Wait for all background jobs

    // Collect results
    for (size_t i = 0; i < exercise.test_cases.size(); ++i) {
        // Read output
        std::stringstream outputFile;
        outputFile << testOutputDir << "/test_" << i << ".txt";

        std::ifstream outFile(outputFile.str().c_str());
        std::string output;
        if (outFile.is_open()) {
            std::getline(outFile, output, '\0');
            outFile.close();
        }

        // Read exit code
        std::stringstream exitFile;
        exitFile << testOutputDir << "/test_" << i << ".exit";

        std::ifstream exitCodeFile(exitFile.str().c_str());
        int exitCode = 0;
        if (exitCodeFile.is_open()) {
            exitCodeFile >> exitCode;
            exitCodeFile.close();
        }

        // Get expected from test case
        const TestCase &tc = exercise.test_cases[i];
        bool passed = (exitCode == 0 && (output == tc.expected || output == tc.expected + "\n"));

        testResultsJson << "  {\n"
                       << "    \"test_case\": " << (i + 1) << ",\n"
                       << "    \"description\": \"" << tc.description << "\",\n"
                       << "    \"passed\": " << (passed ? "true" : "false") << ",\n"
                       << "    \"expected\": \"" << tc.expected << "\",\n"
                       << "    \"actual\": \"" << output << "\"\n"
                       << "  }" << (i < exercise.test_cases.size() - 1 ? "," : "") << "\n";

        if (!passed) {
            _lastError = "Test case " + std::string(1, ('0' + i + 1)) + " failed: expected '" +
                        tc.expected + "', got '" + output + "'";
            _testResults = testResultsJson.str() + "]";
            return false;
        }
    }

    testResultsJson << "]";
    _testResults = testResultsJson.str();

    // Cleanup test output directory
    std::string cleanupCmd = "rm -rf " + testOutputDir;
    system(cleanupCmd.c_str());

    return true;
}

bool Validator::validateTestGeneration(const ExerciseData &exercise) {
    // Generate tester.sh based on exercise type
    std::string testerPath = _tempDir + "/tester.sh";

    std::ofstream testerFile(testerPath.c_str());
    if (!testerFile.is_open()) {
        _lastError = "Failed to create tester.sh";
        return false;
    }

    // Basic tester.sh template
    testerFile << "#!/bin/bash\n\n";
    testerFile << "# Auto-generated test harness for " << exercise.exercise_name << "\n\n";

    if (exercise.exercise_type == "function") {
        testerFile << "# Function exercise - use auto_correc_main.sh template\n";
        testerFile << ". .system/auto_correc_main.sh\n\n";
    } else {
        testerFile << "# Program exercise - use auto_correc_program.sh template\n";
        testerFile << ". .system/auto_correc_program.sh\n\n";
    }

    // Add test cases
    testerFile << "# Test cases\n";
    for (size_t i = 0; i < exercise.test_cases.size(); ++i) {
        const TestCase &tc = exercise.test_cases[i];
        testerFile << "# Test " << (i + 1) << ": " << tc.description << "\n";
        if (!tc.explanation.empty()) {
            testerFile << "# " << tc.explanation << "\n";
        }
        testerFile << "echo \"" << tc.expected << "\" | ./solution";

        // Add arguments
        for (size_t j = 0; j < tc.input.size(); ++j) {
            testerFile << " \"" << tc.input[j] << "\"";
        }
        testerFile << "\n\n";
    }

    testerFile.close();

    // Make executable
    std::string chmodCmd = "chmod +x " + testerPath;
    system(chmodCmd.c_str());

    // Validate bash syntax
    std::string bashCheck = runCommand("bash -n " + testerPath + " 2>&1");
    if (!bashCheck.empty()) {
        _lastError = "Bash syntax error in tester.sh: " + bashCheck;
        return false;
    }

    return true;
}

bool Validator::validateFinal(const ExerciseData &exercise) {
    (void)exercise; // Exercise parameter not used in final validation
    // Run full validation cycle
    std::string executable = _tempDir + "/solution";
    std::string testerPath = _tempDir + "/tester.sh";

    // Check files exist
    if (!fileExists(executable)) {
        _lastError = "Solution executable not found for final validation";
        return false;
    }

    if (!fileExists(testerPath)) {
        _lastError = "tester.sh not found for final validation";
        return false;
    }

    // Run valgrind to check for memory leaks
    std::cout << "   Running memory leak detection with valgrind..." << std::flush;
    std::stringstream valgrindCmd;
    valgrindCmd << "valgrind --leak-check=full --error-exitcode=1 --quiet "
                << executable << " < /dev/null 2>&1";

    std::string valgrindOutput = runCommand(valgrindCmd.str());

    // Check for memory leaks (simplified check)
    if (valgrindOutput.find("ERROR SUMMARY: 0 errors") == std::string::npos &&
        valgrindOutput.find("All heap blocks were freed -- no leaks are possible") == std::string::npos) {
        std::cout << " ❌" << std::endl;
        _lastError = "Memory leaks detected:\n" + valgrindOutput;
        return false;
    }

    std::cout << " ✓" << std::endl;
    std::cout << "   Running infinite loop detection..." << std::flush;

    // Run infinite loop detection (already using timeout in execution testing)
    // This is a final sanity check - run solution with timeout
    std::stringstream timeoutCmd;
    timeoutCmd << "timeout 3s " << executable << " < /dev/null > /dev/null 2>&1";
    int ret = system(timeoutCmd.str().c_str());

    // Timeout returns 124 if timed out, otherwise 0 or other error code
    if (ret == 124) {
        std::cout << " ❌" << std::endl;
        _lastError = "Infinite loop detected: solution timed out";
        return false;
    }

    std::cout << " ✓" << std::endl;
    return true;
}

std::string Validator::runCommand(const std::string &cmd) const {
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return "";
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }

    pclose(pipe);
    return result;
}

bool Validator::fileExists(const std::string &path) const {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}
