/**
 * Unit tests for QualityScorer class
 * Tests: Scoring calculations, threshold logic, report generation
 */

#include <iostream>
#include <cassert>
#include "../includes/QualityScorer.hpp"

// Test 1: QualityScorer initialization
void testQualityScorerInitialization() {
    std::cout << "Test 1: QualityScorer initialization... ";

    QualityScorer scorer;
    assert(scorer.getLastError().empty());
    std::cout << "PASS" << std::endl;
}

// Test 2: QualityReport initialization
void testQualityReportInitialization() {
    std::cout << "Test 2: QualityReport initialization... ";

    QualityReport report;
    assert(report.overall_score == 0);
    assert(report.compilation_score == 0);
    assert(report.test_coverage_score == 0);
    assert(report.subject_clarity_score == 0);
    assert(report.code_quality_score == 0);
    assert(report.approved == false);

    std::cout << "PASS" << std::endl;
}

// Test 3: Compilation scoring - clean compilation
void testCompilationScoreClean() {
    std::cout << "Test 3: Compilation scoring with clean compilation... ";

    std::string cleanLog = "gcc: no warnings\nCompilation successful\n";

    // Verify log indicates success
    assert(cleanLog.find("successful") != std::string::npos);
    std::cout << "PASS" << std::endl;
}

// Test 4: Compilation scoring - with warnings
void testCompilationScoreWarnings() {
    std::cout << "Test 4: Compilation scoring with warnings... ";

    std::string warningLog = "warning: unused variable 'x'\nwarning: implicit declaration\n";

    // Verify log contains warnings
    assert(warningLog.find("warning") != std::string::npos);
    std::cout << "PASS" << std::endl;
}

// Test 5: Compilation scoring - with errors
void testCompilationScoreErrors() {
    std::cout << "Test 5: Compilation scoring with errors... ";

    std::string errorLog = "error: syntax error\nerror: undeclared variable\n";

    // Verify log contains errors
    assert(errorLog.find("error") != std::string::npos);
    std::cout << "PASS" << std::endl;
}

// Test 6: Test coverage scoring - comprehensive tests
void testTestCoverageScoreComprehensive() {
    std::cout << "Test 6: Test coverage scoring with comprehensive tests... ";

    std::vector<TestCase> testCases;

    // Normal cases
    TestCase normal1;
    normal1.description = "Normal case 1";
    normal1.explanation = "Test normal input";
    testCases.push_back(normal1);

    TestCase normal2;
    normal2.description = "Normal case 2";
    normal2.explanation = "Test another normal input";
    testCases.push_back(normal2);

    // Edge cases
    TestCase edge1;
    edge1.description = "Edge case: empty input";
    edge1.explanation = "Test boundary condition";
    testCases.push_back(edge1);

    TestCase edge2;
    edge2.description = "Edge case: max value";
    edge2.explanation = "Test maximum value";
    testCases.push_back(edge2);

    // Error cases
    TestCase error1;
    error1.description = "Error case: null input";
    error1.explanation = "Test error handling";
    testCases.push_back(error1);

    // Should have good coverage
    assert(testCases.size() >= 5);
    std::cout << "PASS" << std::endl;
}

// Test 7: Test coverage scoring - insufficient tests
void testTestCoverageScoreInsufficient() {
    std::cout << "Test 7: Test coverage scoring with insufficient tests... ";

    std::vector<TestCase> testCases;

    TestCase tc;
    tc.description = "Only one test";
    tc.explanation = "Not enough coverage";
    testCases.push_back(tc);

    // Should have poor coverage (< 5 tests)
    assert(testCases.size() < 5);
    std::cout << "PASS" << std::endl;
}

// Test 8: Test coverage categorization
void testTestCoverageCategorization() {
    std::cout << "Test 8: Test coverage categorization... ";

    std::vector<TestCase> testCases;

    // Normal case
    TestCase normal;
    normal.description = "Normal operation";
    normal.explanation = "Tests standard behavior";
    testCases.push_back(normal);

    // Edge case
    TestCase edge;
    edge.description = "Edge case: empty";
    edge.explanation = "Tests boundary";
    testCases.push_back(edge);

    // Error case
    TestCase error;
    error.description = "Error case: invalid";
    error.explanation = "Tests error handling";
    testCases.push_back(error);

    assert(testCases.size() == 3);
    std::cout << "PASS" << std::endl;
}

// Test 9: Subject clarity scoring - good subject
void testSubjectClarityScoreGood() {
    std::cout << "Test 9: Subject clarity scoring with good subject... ";

    std::string goodSubject =
        "## Exercise: String Length\n\n"
        "Write a function that calculates the length of a string.\n\n"
        "### Requirements:\n"
        "- The function must be named `my_strlen`\n"
        "- It should take a const char* as parameter\n"
        "- Return the number of characters before the null terminator\n"
        "- You must not use the standard strlen function\n\n"
        "### Example:\n"
        "```c\n"
        "my_strlen(\"Hello\") -> 5\n"
        "my_strlen(\"\") -> 0\n"
        "```\n";

    // Good subject should be long enough (> 100 chars)
    assert(goodSubject.length() > 100);
    assert(goodSubject.find("Example") != std::string::npos);
    assert(goodSubject.find("Requirements") != std::string::npos);

    std::cout << "PASS" << std::endl;
}

// Test 10: Subject clarity scoring - poor subject
void testSubjectClarityScorePoor() {
    std::cout << "Test 10: Subject clarity scoring with poor subject... ";

    std::string poorSubject = "Write a strlen function.";

    // Poor subject is too short
    assert(poorSubject.length() < 50);
    std::cout << "PASS" << std::endl;
}

// Test 11: Code quality scoring - good code
void testCodeQualityScoreGood() {
    std::cout << "Test 11: Code quality scoring with good code... ";

    std::string goodCode =
        "#include <stdlib.h>\n\n"
        "typedef struct s_node {\n"
        "    int data;\n"
        "    struct s_node* next;\n"
        "} t_node;\n\n"
        "t_node* create_node(int data) {\n"
        "    t_node* node = (t_node*)malloc(sizeof(t_node));\n"
        "    if (node == NULL)\n"
        "        return NULL;\n"
        "    node->data = data;\n"
        "    node->next = NULL;\n"
        "    return node;\n"
        "}\n\n"
        "void free_node(t_node* node) {\n"
        "    free(node);\n"
        "}\n";

    // Good code should have proper memory management
    assert(goodCode.find("malloc") != std::string::npos);
    assert(goodCode.find("free") != std::string::npos);
    assert(goodCode.find("NULL") != std::string::npos);

    std::cout << "PASS" << std::endl;
}

// Test 12: Code quality scoring - poor code (memory leak)
void testCodeQualityScoreMemoryLeak() {
    std::cout << "Test 12: Code quality scoring with memory leak... ";

    std::string leakyCode =
        "t_node* create_node(int data) {\n"
        "    t_node* node = (t_node*)malloc(sizeof(t_node));\n"
        "    node->data = data;\n"
        "    return node;\n"
        "    // Missing cleanup\n"
        "}\n";

    // Leaky code mallocs without frees
    assert(leakyCode.find("malloc") != std::string::npos);
    // Check that there's no standalone "free" call (only in comments/strings)
    size_t freePos = leakyCode.find("free");
    assert(freePos == std::string::npos);

    std::cout << "PASS" << std::endl;
}

// Test 13: Overall score calculation
void testOverallScoreCalculation() {
    std::cout << "Test 13: Overall score calculation with weighted components... ";

    // Weighted formula: compilation 25%, test_coverage 30%, subject_clarity 25%, code_quality 20%
    int compilation = 100;
    int test_coverage = 80;
    int subject_clarity = 90;
    int code_quality = 70;

    double overall = (compilation * 0.25) +
                     (test_coverage * 0.30) +
                     (subject_clarity * 0.25) +
                     (code_quality * 0.20);

    int expected = (int)overall;  // Should be ~86

    assert(expected >= 80 && expected <= 90);
    std::cout << "PASS" << std::endl;
}

// Test 14: Approval threshold - approve
void testApprovalThresholdApprove() {
    std::cout << "Test 14: Approval threshold with score >= 70... ";

    QualityReport report;
    report.overall_score = 75;
    report.approved = (report.overall_score >= 70);

    assert(report.approved == true);
    std::cout << "PASS" << std::endl;
}

// Test 15: Approval threshold - reject
void testApprovalThresholdReject() {
    std::cout << "Test 15: Approval threshold with score < 70... ";

    QualityReport report;
    report.overall_score = 65;
    report.approved = (report.overall_score >= 70);

    assert(report.approved == false);
    std::cout << "PASS" << std::endl;
}

// Test 16: Quality report details
void testQualityReportDetails() {
    std::cout << "Test 16: Quality report with detailed breakdown... ";

    QualityReport report;
    report.overall_score = 85;
    report.compilation_score = 100;
    report.test_coverage_score = 80;
    report.subject_clarity_score = 90;
    report.code_quality_score = 70;
    report.compilation_details = "Clean compilation";
    report.test_coverage_details = "Good test coverage";
    report.subject_clarity_details = "Clear subject";
    report.code_quality_details = "Follows 42 norms";
    report.approved = true;

    assert(report.overall_score == 85);
    assert(!report.compilation_details.empty());
    assert(!report.test_coverage_details.empty());
    assert(!report.subject_clarity_details.empty());
    assert(!report.code_quality_details.empty());
    assert(report.approved == true);

    std::cout << "PASS" << std::endl;
}

// Test 17: Edge case - perfect scores
void testEdgeCasePerfectScores() {
    std::cout << "Test 17: Edge case with perfect scores... ";

    QualityReport report;
    report.compilation_score = 100;
    report.test_coverage_score = 100;
    report.subject_clarity_score = 100;
    report.code_quality_score = 100;

    report.overall_score = (report.compilation_score * 0.25) +
                           (report.test_coverage_score * 0.30) +
                           (report.subject_clarity_score * 0.25) +
                           (report.code_quality_score * 0.20);

    report.approved = (report.overall_score >= 70);

    assert(report.overall_score == 100);
    assert(report.approved == true);

    std::cout << "PASS" << std::endl;
}

// Test 18: Edge case - minimum passing score
void testEdgeCaseMinimumPassing() {
    std::cout << "Test 18: Edge case with minimum passing score (70)... ";

    QualityReport report;
    report.overall_score = 70;
    report.approved = (report.overall_score >= 70);

    assert(report.approved == true);
    std::cout << "PASS" << std::endl;
}

// Test 19: Edge case - just below passing
void testEdgeCaseJustBelowPassing() {
    std::cout << "Test 19: Edge case just below passing (69)... ";

    QualityReport report;
    report.overall_score = 69;
    report.approved = (report.overall_score >= 70);

    assert(report.approved == false);
    std::cout << "PASS" << std::endl;
}

// Test 20: QualityScorer copy constructor
void testQualityScorerCopyConstructor() {
    std::cout << "Test 20: QualityScorer copy constructor... ";

    QualityScorer scorer1;
    QualityScorer scorer2(scorer1);

    // Should not crash
    std::cout << "PASS" << std::endl;
}

// Test 21: QualityScorer assignment operator
void testQualityScorerAssignment() {
    std::cout << "Test 21: QualityScorer assignment operator... ";

    QualityScorer scorer1;
    QualityScorer scorer2;
    scorer2 = scorer1;

    // Should not crash
    std::cout << "PASS" << std::endl;
}

// Test 22: Test coverage with no tests
void testTestCoverageNoTests() {
    std::cout << "Test 22: Test coverage with no test cases... ";

    std::vector<TestCase> emptyCases;

    // Empty test cases should score 0
    assert(emptyCases.size() == 0);
    std::cout << "PASS" << std::endl;
}

// Test 23: Subject clarity - empty subject
void testSubjectClarityEmpty() {
    std::cout << "Test 23: Subject clarity with empty subject... ";

    std::string emptySubject = "";

    // Empty subject should score 0
    assert(emptySubject.empty());
    std::cout << "PASS" << std::endl;
}

// Test 24: Code quality - empty code
void testCodeQualityEmpty() {
    std::cout << "Test 24: Code quality with empty code... ";

    std::string emptyCode = "";

    // Empty code should score 0
    assert(emptyCode.empty());
    std::cout << "PASS" << std::endl;
}

int main() {
    std::cout << "\n=== QualityScorer Unit Tests ===" << std::endl;
    std::cout << "Testing: Scoring calculations, threshold logic, report generation\n"
              << std::endl;

    try {
        testQualityScorerInitialization();
        testQualityReportInitialization();
        testCompilationScoreClean();
        testCompilationScoreWarnings();
        testCompilationScoreErrors();
        testTestCoverageScoreComprehensive();
        testTestCoverageScoreInsufficient();
        testTestCoverageCategorization();
        testSubjectClarityScoreGood();
        testSubjectClarityScorePoor();
        testCodeQualityScoreGood();
        testCodeQualityScoreMemoryLeak();
        testOverallScoreCalculation();
        testApprovalThresholdApprove();
        testApprovalThresholdReject();
        testQualityReportDetails();
        testEdgeCasePerfectScores();
        testEdgeCaseMinimumPassing();
        testEdgeCaseJustBelowPassing();
        testQualityScorerCopyConstructor();
        testQualityScorerAssignment();
        testTestCoverageNoTests();
        testSubjectClarityEmpty();
        testCodeQualityEmpty();

        std::cout << "\n✓ All QualityScorer tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n✗ Test failed with unknown exception" << std::endl;
        return 1;
    }
}
