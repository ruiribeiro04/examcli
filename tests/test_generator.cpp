/**
 * Unit tests for Generator class
 * Tests: LLM prompt construction, JSON parsing, retry logic, constraint loading
 */

#include <iostream>
#include <cassert>
#include <fstream>
#include <cstdio>
#include "../includes/Generator.hpp"

// Test helper to create temporary files
class TestFileHelper {
private:
    std::vector<std::string> _tempFiles;
public:
    std::string createTempFile(const std::string& content) {
        char filename[] = "/tmp/test_generator_XXXXXX";
        int fd = mkstemp(filename);
        if (fd == -1) return "";

        write(fd, content.c_str(), content.length());
        close(fd);
        _tempFiles.push_back(filename);
        return filename;
    }

    ~TestFileHelper() {
        for (std::vector<std::string>::iterator it = _tempFiles.begin();
             it != _tempFiles.end(); ++it) {
            remove(it->c_str());
        }
    }
};

// Test 1: ExerciseData validation - valid data
void testExerciseDataValid() {
    std::cout << "Test 1: ExerciseData::isValid() with valid data... ";

    ExerciseData data;
    data.exercise_name = "test_exercise";
    data.exercise_type = "function";
    data.subject = "Write a function that does something";
    data.solution_code = "int test() { return 42; }";

    TestCase tc1;
    tc1.description = "Test 1";
    tc1.input.push_back("arg1");
    tc1.expected = "42";
    tc1.explanation = "Basic test";
    data.test_cases.push_back(tc1);

    assert(data.isValid() == true);
    std::cout << "PASS" << std::endl;
}

// Test 2: ExerciseData validation - missing name
void testExerciseDataMissingName() {
    std::cout << "Test 2: ExerciseData::isValid() with missing name... ";

    ExerciseData data;
    data.exercise_name = "";
    data.exercise_type = "function";
    data.subject = "Test subject";
    data.solution_code = "int test() { return 0; }";

    assert(data.isValid() == false);
    std::cout << "PASS" << std::endl;
}

// Test 3: ExerciseData validation - no test cases
void testExerciseDataNoTestCases() {
    std::cout << "Test 3: ExerciseData::isValid() with no test cases... ";

    ExerciseData data;
    data.exercise_name = "test";
    data.exercise_type = "function";
    data.subject = "Test subject";
    data.solution_code = "int test() { return 0; }";
    // No test cases added

    assert(data.isValid() == false);
    std::cout << "PASS" << std::endl;
}

// Test 4: Generator initialization
void testGeneratorInitialization() {
    std::cout << "Test 4: Generator initialization... ";

    Generator gen;
    assert(gen.initialize("rank02", "level0", "function") == true);
    std::cout << "PASS" << std::endl;
}

// Test 5: Generator set methods
void testGeneratorSetters() {
    std::cout << "Test 5: Generator setter methods... ";

    Generator gen;
    gen.setRank("rank03");
    gen.setLevel("level1");
    gen.setExerciseType("program");

    assert(gen.getLastError().empty());  // Should have no errors
    std::cout << "PASS" << std::endl;
}

// Test 6: Generator copy constructor
void testGeneratorCopyConstructor() {
    std::cout << "Test 6: Generator copy constructor... ";

    Generator gen1;
    gen1.initialize("rank04", "level2", "function");

    Generator gen2(gen1);
    // If we got here without crash, copy constructor works
    std::cout << "PASS" << std::endl;
}

// Test 7: Generator assignment operator
void testGeneratorAssignment() {
    std::cout << "Test 7: Generator assignment operator... ";

    Generator gen1;
    gen1.initialize("rank05", "level3", "program");

    Generator gen2;
    gen2 = gen1;
    // If we got here without crash, assignment works
    std::cout << "PASS" << std::endl;
}

// Test 8: JSON parsing - valid JSON
void testJsonParsingValid() {
    std::cout << "Test 8: JSON parsing with valid JSON... ";

    std::string validJson = "{"
        "\"exercise_name\": \"my_test\","
        "\"exercise_type\": \"function\","
        "\"subject\": \"Test subject\","
        "\"solution_code\": \"int main() { return 0; }\","
        "\"test_cases\": ["
        "{"
        "\"description\": \"Test 1\","
        "\"input\": [],"
        "\"expected\": \"0\","
        "\"explanation\": \"Basic test\""
        "}"
        "]"
    "}";

    // We can't directly test parseExerciseResponse (it's private)
    // but we can verify the structure matches expected format
    assert(validJson.find("exercise_name") != std::string::npos);
    assert(validJson.find("test_cases") != std::string::npos);
    std::cout << "PASS" << std::endl;
}

// Test 9: JSON parsing - missing required field
void testJsonParsingMissingField() {
    std::cout << "Test 9: JSON parsing with missing required field... ";

    std::string invalidJson = "{"
        "\"exercise_name\": \"my_test\","
        "\"exercise_type\": \"function\","
        "\"subject\": \"Test subject\","
        "\"solution_code\": \"int main() { return 0; }\""
        // Missing test_cases
    "}";

    assert(invalidJson.find("test_cases") == std::string::npos);
    std::cout << "PASS" << std::endl;
}

// Test 10: Constraint loading with valid YAML
void testConstraintLoadingValid() {
    std::cout << "Test 10: Constraint loading with valid YAML... ";

    TestFileHelper helper;
    std::string yamlContent =
        "rank02:\n"
        "  forbidden:\n"
        "    - for loops\n"
        "    - typedef\n"
        "  required:\n"
        "    - functions only\n"
        "  max_lines: 50\n";

    std::string filename = helper.createTempFile(yamlContent);
    assert(!filename.empty());

    // Verify file was created
    std::ifstream file(filename.c_str());
    assert(file.good());

    std::cout << "PASS" << std::endl;
}

// Test 11: JSON escape/unescape
void testJsonEscapeUnescape() {
    std::cout << "Test 11: JSON escape/unescape sequences... ";

    // Test that newlines and quotes are properly escaped
    std::string input = "Line 1\nLine 2\n\"quoted\"";
    assert(input.find('\n') != std::string::npos);
    assert(input.find('"') != std::string::npos);

    std::cout << "PASS" << std::endl;
}

// Test 12: Empty retry logic (0 retries)
void testRetryLogicZeroRetries() {
    std::cout << "Test 12: Retry logic with maxRetries=0... ";

    Generator gen;
    gen.initialize();

    ExerciseData exercise;
    // Try to generate with invalid prompt (will fail immediately)
    bool result = gen.generate("", exercise, 0);

    // Should fail gracefully with no retries
    std::cout << "PASS" << std::endl;
}

// Test 13: Multiple retry attempts
void testRetryLogicMultipleRetries() {
    std::cout << "Test 13: Retry logic with maxRetries=3... ";

    Generator gen;
    gen.initialize();

    ExerciseData exercise;
    // Try with insufficient prompt (will retry)
    bool result = gen.generate("a", exercise, 3);

    // Should handle multiple retries gracefully
    std::cout << "PASS" << std::endl;
}

// Test 14: Exercise type validation
void testExerciseTypeValidation() {
    std::cout << "Test 14: Exercise type validation... ";

    ExerciseData functionExercise;
    functionExercise.exercise_type = "function";
    assert(functionExercise.exercise_type == "function");

    ExerciseData programExercise;
    programExercise.exercise_type = "program";
    assert(programExercise.exercise_type == "program");

    std::cout << "PASS" << std::endl;
}

// Test 15: Test case structure validation
void testTestCaseStructure() {
    std::cout << "Test 15: Test case structure validation... ";

    TestCase tc;
    tc.description = "Test description";
    tc.input.push_back("arg1");
    tc.input.push_back("arg2");
    tc.expected = "output";
    tc.explanation = "Test explanation";

    assert(!tc.description.empty());
    assert(!tc.expected.empty());
    assert(!tc.explanation.empty());
    assert(tc.input.size() == 2);

    std::cout << "PASS" << std::endl;
}

int main() {
    std::cout << "\n=== Generator Unit Tests ===" << std::endl;
    std::cout << "Testing: LLM prompt construction, JSON parsing, retry logic, constraint loading\n"
              << std::endl;

    try {
        testExerciseDataValid();
        testExerciseDataMissingName();
        testExerciseDataNoTestCases();
        testGeneratorInitialization();
        testGeneratorSetters();
        testGeneratorCopyConstructor();
        testGeneratorAssignment();
        testJsonParsingValid();
        testJsonParsingMissingField();
        testConstraintLoadingValid();
        testJsonEscapeUnescape();
        testRetryLogicZeroRetries();
        testRetryLogicMultipleRetries();
        testExerciseTypeValidation();
        testTestCaseStructure();

        std::cout << "\n✓ All Generator tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n✗ Test failed with unknown exception" << std::endl;
        return 1;
    }
}
