#include "Generator.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

Generator::Generator() : _rank("rank02"), _level("level0"),
    _exerciseType("function"), _constraints(), _lastError() {
}

Generator::Generator(const Generator &other)
    : _llmClient(other._llmClient), _rank(other._rank), _level(other._level),
      _exerciseType(other._exerciseType), _constraints(other._constraints),
      _lastError(other._lastError) {
}

Generator &Generator::operator=(const Generator &other) {
    if (this != &other) {
        _llmClient = other._llmClient;
        _rank = other._rank;
        _level = other._level;
        _exerciseType = other._exerciseType;
        _constraints = other._constraints;
        _lastError = other._lastError;
    }
    return *this;
}

Generator::~Generator() {
}

bool Generator::initialize(const std::string &rank,
                          const std::string &level,
                          const std::string &type) {
    if (!_llmClient.initialize()) {
        _lastError = _llmClient.getLastError();
        return false;
    }

    _rank = rank;
    _level = level;
    _exerciseType = type;

    // Load constraints from constraints.yaml if it exists
    loadConstraints("constraints.yaml");

    return true;
}

void Generator::setRank(const std::string &rank) {
    _rank = rank;
}

void Generator::setLevel(const std::string &level) {
    _level = level;
}

void Generator::setExerciseType(const std::string &type) {
    _exerciseType = type;
}

bool Generator::generate(const std::string &prompt, ExerciseData &exercise,
                        int maxRetries) {
    // Validate prompt
    if (prompt.empty() || prompt.length() < 5) {
        _lastError = "Prompt too short. Please provide a more detailed exercise description (at least 5 characters).";
        return false;
    }

    std::string fullPrompt = buildGenerationPrompt(prompt);
    int retryCount = 0;
    std::string lastError;

    // Estimate cost before generation (task 11.5)
    int estimatedInputTokens = estimateTokenCount(fullPrompt);
    int estimatedOutputTokens = 2000;  // Typical exercise response
    float estimatedCost = estimateCost(estimatedInputTokens, estimatedOutputTokens);

    std::cout << "🎯 Generating exercise..." << std::endl;
    std::cout << "📝 Prompt: " << prompt << std::endl;
    std::cout << "💰 Estimated cost: $" << estimatedCost << " ("
             << estimatedInputTokens << " input + ~" << estimatedOutputTokens
             << " output tokens)" << std::endl;

    while (retryCount <= maxRetries) {
        if (retryCount > 0) {
            std::cout << "⚠️  Retry " << retryCount << "/" << maxRetries
                     << " - Refining exercise..." << std::endl;

            // Build refinement prompt with error context
            fullPrompt = buildRefinementPrompt(lastError, "generation");
        }

        // Call LLM to generate exercise
        std::string response;
        if (!_llmClient.complete(fullPrompt, response)) {
            std::string llmError = _llmClient.getLastError();

            // Provide helpful error messages based on common issues
            if (llmError.find("timeout") != std::string::npos ||
                llmError.find("connection") != std::string::npos) {
                lastError = "Network error: Unable to reach LLM API. Please check your internet connection.";
            } else if (llmError.find("401") != std::string::npos ||
                       llmError.find("403") != std::string::npos) {
                lastError = "Authentication error: Invalid API key. Please check LLM_API_KEY environment variable.";
            } else if (llmError.find("429") != std::string::npos) {
                lastError = "Rate limit exceeded: Too many requests. Please wait a moment and try again.";
            } else if (llmError.find("500") != std::string::npos ||
                       llmError.find("502") != std::string::npos ||
                       llmError.find("503") != std::string::npos) {
                lastError = "LLM service error: The API is experiencing issues. Please try again later.";
            } else {
                lastError = "LLM API error: " + llmError;
            }

            _lastError = lastError;
            std::cout << "❌ " << lastError << std::endl;
            retryCount++;
            continue;
        }

        // Check if response is empty
        if (response.empty()) {
            lastError = "LLM returned empty response. This may indicate an API issue.";
            std::cout << "❌ " << lastError << std::endl;
            retryCount++;
            continue;
        }

        // Parse the JSON response (markdown already removed by LLMClient)
        if (!parseExerciseResponse(response, exercise)) {
            lastError = "Failed to parse exercise JSON. The LLM response may not be in the correct format.";
            std::cout << "❌ " << lastError << std::endl;
            std::cout << "💡 Tip: Try rephrasing your prompt to be more specific." << std::endl;
            retryCount++;
            continue;
        }

        // Validate the exercise data
        if (!exercise.isValid()) {
            lastError = "Invalid exercise data: missing required fields (exercise_name, exercise_type, subject, solution_code, or test_cases)";
            std::cout << "❌ " << lastError << std::endl;
            retryCount++;
            continue;
        }

        std::cout << "✓ Exercise generated: " << exercise.exercise_name << std::endl;
        return true;
    }

    // All retries exhausted
    _lastError = "Maximum retries exceeded. Last error: " + lastError;
    std::cout << "\n❌ Generation failed after " << maxRetries << " attempts." << std::endl;
    std::cout << "💡 Suggestions:" << std::endl;
    std::cout << "   - Try rephrasing your prompt to be more specific" << std::endl;
    std::cout << "   - Simplify the exercise requirements" << std::endl;
    std::cout << "   - Check your LLM API configuration" << std::endl;
    std::cout << "   - Try again later if the API is experiencing issues" << std::endl;
    return false;
}

const std::string &Generator::getLastError() const {
    return _lastError;
}

std::string Generator::buildGenerationPrompt(const std::string &userPrompt) const {
    std::stringstream prompt;

    // Optimized prompt (task 12.3) - more concise to reduce token usage
    prompt << "Generate 42-style C exercise: " << userPrompt << "\n\n"
           << "Rank: " << _rank << " | Level: " << _level << " | Type: " << _exerciseType << "\n";

    // Add rank-specific constraints if present
    std::string constraints = getConstraintsForRank(_rank);
    if (!constraints.empty()) {
        prompt << "Constraints: " << constraints << "\n";
    }

    prompt << "\nRespond with valid JSON only (no markdown):\n"
           << "{\n"
           << "  \"exercise_name\": \"snake_case_name\",\n"
           << "  \"exercise_type\": \"function|program\",\n"
           << "  \"subject\": \"description with examples\",\n"
           << "  \"solution_code\": \"full C code with main()\",\n"
           << "  \"test_cases\": [{\n"
           << "    \"description\": \"test purpose\",\n"
           << "    \"input\": [\"arg1\", \"arg2\"],\n"
           << "    \"expected\": \"output\",\n"
           << "    \"explanation\": \"why this matters\"\n"
           << "  }]\n"
           << "}\n\n"
           << "Requirements:\n"
           << "- Must compile: -std=c99 -Wall -Wextra -Werror\n"
           << "- 42 norms: " << (_rank == "rank02" ? "recursion only, no loops" : "proper memory management") << "\n"
           << "- Tests: 2+ normal, 2+ edge, 1+ error cases\n"
           << "- main(): read argv, print result to stdout only\n"
           << "- Raw JSON, no markdown, exact field names above";

    return prompt.str();
}

std::string Generator::buildRefinementPrompt(const std::string &error,
                                            const std::string &stage) const {
    std::stringstream prompt;

    // Optimized refinement prompt (task 12.3)
    prompt << "Fix exercise (" << stage << " error): " << error << "\n\n"
           << "Respond with complete corrected JSON only.\n"
           << "Use exact fields: exercise_name, exercise_type, subject, solution_code, test_cases\n"
           << "Raw JSON, no markdown.";

    return prompt.str();
}

bool Generator::parseExerciseResponse(const std::string &json,
                                     ExerciseData &exercise) {
    // Pre-process: convert literal escape sequences to actual characters
    // The LLM may return JSON with literal \n, \t, \" etc. as two characters
    std::string processedJson;

    for (size_t i = 0; i < json.length(); ++i) {
        if (json[i] == '\\' && i + 1 < json.length()) {
            // We have a backslash, check if it's an escape sequence
            char next = json[i + 1];
            switch (next) {
                case 'n':
                    processedJson += '\n';
                    i++; // skip the 'n'
                    break;
                case 't':
                    processedJson += '\t';
                    i++; // skip the 't'
                    break;
                case 'r':
                    processedJson += '\r';
                    i++; // skip the 'r'
                    break;
                case '\\':
                    processedJson += '\\';
                    i++; // skip the second backslash
                    break;
                case '"':
                    processedJson += '"';
                    i++; // skip the quote
                    break;
                default:
                    // Not a recognized escape, keep the backslash
                    processedJson += json[i];
                    break;
            }
        } else {
            processedJson += json[i];
        }
    }

    // Now parse the processed JSON
    // Parse exercise_name
    size_t namePos = processedJson.find("\"exercise_name\":");
    if (namePos != std::string::npos) {
        size_t start = processedJson.find('"', namePos + 16) + 1;
        size_t end = processedJson.find('"', start);
        if (start != std::string::npos && end != std::string::npos) {
            exercise.exercise_name = processedJson.substr(start, end - start);
        }
    }

    // Parse exercise_type
    size_t typePos = processedJson.find("\"exercise_type\":");
    if (typePos != std::string::npos) {
        size_t start = processedJson.find('"', typePos + 16) + 1;
        size_t end = processedJson.find('"', start);
        if (start != std::string::npos && end != std::string::npos) {
            exercise.exercise_type = processedJson.substr(start, end - start);
        }
    }

    // Parse subject
    size_t subjectPos = processedJson.find("\"subject\":");
    if (subjectPos != std::string::npos) {
        size_t start = processedJson.find('"', subjectPos + 10) + 1;
        size_t end = start;
        while (end < processedJson.length()) {
            if (processedJson[end] == '"' && processedJson[end - 1] != '\\') {
                break;
            }
            end++;
        }
        if (end < processedJson.length()) {
            exercise.subject = unescapeJson(processedJson.substr(start, end - start));
        }
    }

    // Parse solution_code
    size_t codePos = processedJson.find("\"solution_code\":");
    if (codePos != std::string::npos) {
        size_t start = processedJson.find('"', codePos + 16) + 1;
        size_t end = start;
        while (end < processedJson.length()) {
            if (processedJson[end] == '"' && processedJson[end - 1] != '\\') {
                break;
            }
            end++;
        }
        if (end < processedJson.length()) {
            exercise.solution_code = unescapeJson(processedJson.substr(start, end - start));
        }
    }

    // Parse test_cases array
    size_t testCasesPos = processedJson.find("\"test_cases\":");
    if (testCasesPos != std::string::npos) {
        size_t arrayStart = processedJson.find('[', testCasesPos);
        if (arrayStart != std::string::npos) {
            size_t current = arrayStart + 1;
            int bracketCount = 1; // We're inside the test_cases array

            while (current < processedJson.length() && bracketCount > 0) {
                if (processedJson[current] == '{' && bracketCount == 1) {
                    // Found a test case object
                    size_t objStart = current;
                    size_t objEnd = current + 1;
                    int objBraceCount = 1;

                    while (objEnd < processedJson.length() && objBraceCount > 0) {
                        if (processedJson[objEnd] == '{') objBraceCount++;
                        else if (processedJson[objEnd] == '}') objBraceCount--;
                        objEnd++;
                    }

                    if (objBraceCount == 0) {
                        std::string testCaseObj = processedJson.substr(objStart, objEnd - objStart);
                        TestCase testCase;

                        // Parse description
                        size_t descPos = testCaseObj.find("\"description\":");
                        if (descPos != std::string::npos) {
                            size_t start = testCaseObj.find('"', descPos + 14) + 1;
                            size_t end = start;
                            while (end < testCaseObj.length()) {
                                if (testCaseObj[end] == '"' && testCaseObj[end - 1] != '\\') break;
                                end++;
                            }
                            if (end < testCaseObj.length()) {
                                testCase.description = testCaseObj.substr(start, end - start);
                            }
                        }

                        // Parse input array (simplified - just grab the whole array as string)
                        size_t inputPos = testCaseObj.find("\"input\":");
                        if (inputPos != std::string::npos) {
                            size_t arrStart = testCaseObj.find('[', inputPos);
                            if (arrStart != std::string::npos) {
                                size_t arrEnd = testCaseObj.find(']', arrStart);
                                if (arrEnd != std::string::npos) {
                                    // Extract array elements
                                    std::string inputArray = testCaseObj.substr(arrStart, arrEnd - arrStart + 1);
                                    size_t elemStart = 0;
                                    while (elemStart < inputArray.length()) {
                                        size_t quoteStart = inputArray.find('"', elemStart);
                                        if (quoteStart == std::string::npos || quoteStart > arrEnd) break;
                                        size_t quoteEnd = quoteStart + 1;
                                        while (quoteEnd < inputArray.length() &&
                                               (inputArray[quoteEnd] != '"' || inputArray[quoteEnd - 1] == '\\')) {
                                            quoteEnd++;
                                        }
                                        if (quoteEnd < inputArray.length()) {
                                            std::string inputVal = inputArray.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                                            testCase.input.push_back(inputVal);
                                            elemStart = quoteEnd + 1;
                                        } else {
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        // Parse expected
                        size_t expectedPos = testCaseObj.find("\"expected\":");
                        if (expectedPos != std::string::npos) {
                            size_t start = testCaseObj.find('"', expectedPos + 11) + 1;
                            size_t end = start;
                            while (end < testCaseObj.length()) {
                                if (testCaseObj[end] == '"' && testCaseObj[end - 1] != '\\') break;
                                end++;
                            }
                            if (end < testCaseObj.length()) {
                                testCase.expected = testCaseObj.substr(start, end - start);
                            }
                        }

                        // Parse explanation
                        size_t explPos = testCaseObj.find("\"explanation\":");
                        if (explPos != std::string::npos) {
                            size_t start = testCaseObj.find('"', explPos + 13) + 1;
                            size_t end = start;
                            while (end < testCaseObj.length()) {
                                if (testCaseObj[end] == '"' && testCaseObj[end - 1] != '\\') break;
                                end++;
                            }
                            if (end < testCaseObj.length()) {
                                testCase.explanation = testCaseObj.substr(start, end - start);
                            }
                        }

                        exercise.test_cases.push_back(testCase);
                        current = objEnd;
                    }
                } else if (processedJson[current] == '[') {
                    bracketCount++;
                } else if (processedJson[current] == ']') {
                    bracketCount--;
                }
                current++;
            }
        }
    }

    return exercise.isValid();
}

bool Generator::loadConstraints(const std::string &constraintsFile) {
    std::ifstream file(constraintsFile.c_str());
    if (!file.is_open()) {
        // File is optional, return true if not found
        return true;
    }

    // Simple YAML parsing (very basic)
    // In production, use a proper YAML parser
    std::string line;
    std::string currentRank;

    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Check for rank declaration (e.g., "rank02:")
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos && line[0] != ' ') {
            currentRank = line.substr(0, colonPos);
            continue;
        }

        // Parse constraints (simplified)
        if (!currentRank.empty() && line.find("forbidden:") != std::string::npos) {
            // Would parse forbidden items here
        }
    }

    file.close();
    return true;
}

std::string Generator::getConstraintsForRank(const std::string &rank) const {
    std::map<std::string, std::vector<std::string> >::const_iterator it =
        _constraints.find(rank);

    if (it == _constraints.end()) {
        // Return default constraints
        if (rank == "rank02") {
            return "- No for loops\n- No do-while loops\n- Use recursion\n- No typedef\n- No switch statements";
        }
        return "- Follow 42 norm\n- Proper memory management\n- Error handling";
    }

    std::stringstream ss;
    const std::vector<std::string> &constraints = it->second;
    for (size_t i = 0; i < constraints.size(); ++i) {
        ss << "- " << constraints[i];
        if (i < constraints.size() - 1) {
            ss << "\n";
        }
    }
    return ss.str();
}

std::string Generator::escapeJson(const std::string &input) const {
    std::stringstream escaped;
    escaped << '"';
    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        switch (c) {
        case '"':
            escaped << "\\\"";
            break;
        case '\\':
            escaped << "\\\\";
            break;
        case '\b':
            escaped << "\\b";
            break;
        case '\f':
            escaped << "\\f";
            break;
        case '\n':
            escaped << "\\n";
            break;
        case '\r':
            escaped << "\\r";
            break;
        case '\t':
            escaped << "\\t";
            break;
        default:
            if (c >= 0x20) {
                escaped << c;
            }
            break;
        }
    }
    escaped << '"';
    return escaped.str();
}

std::string Generator::unescapeJson(const std::string &input) const {
    std::stringstream unescaped;
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '\\' && i + 1 < input.length()) {
            switch (input[i + 1]) {
                case 'n': unescaped << '\n'; i++; break;
                case 't': unescaped << '\t'; i++; break;
                case 'r': unescaped << '\r'; i++; break;
                case '\\': unescaped << '\\'; i++; break;
                case '"': unescaped << '"'; i++; break;
                case '/': unescaped << '/'; i++; break;
                case 'b': unescaped << '\b'; i++; break;
                case 'f': unescaped << '\f'; i++; break;
                case 'u': {
                    // Unicode escape \uXXXX
                    if (i + 5 < input.length()) {
                        std::string hexStr = input.substr(i + 2, 4);
                        unsigned int codePoint;
                        std::stringstream ss;
                        ss << std::hex << hexStr;
                        ss >> codePoint;
                        unescaped << static_cast<char>(codePoint);
                        i += 5;
                    } else {
                        unescaped << input[i];
                    }
                    break;
                }
                default:
                    // Unknown escape, keep as-is
                    unescaped << input[i] << input[i + 1];
                    i++;
                    break;
            }
        } else {
            unescaped << input[i];
        }
    }
    return unescaped.str();
}

bool ExerciseData::isValid() const {
    return !exercise_name.empty() &&
           !exercise_type.empty() &&
           !subject.empty() &&
           !solution_code.empty() &&
           (exercise_type == "function" || exercise_type == "program");
}

// Cost estimation methods (task 11.5)
int Generator::estimateTokenCount(const std::string &text) {
    // Rough estimation: ~4 characters per token for English text
    // This is a heuristic approximation
    if (text.empty()) return 0;

    // Count words (separated by spaces/newlines)
    int wordCount = 0;
    bool inWord = false;

    for (size_t i = 0; i < text.length(); i++) {
        if (text[i] == ' ' || text[i] == '\n' || text[i] == '\t') {
            if (inWord) {
                wordCount++;
                inWord = false;
            }
        } else {
            inWord = true;
        }
    }
    if (inWord) wordCount++;

    // Estimate tokens: roughly 0.75 * wordCount for code, 1.0 * wordCount for text
    // Use average of 0.85
    return static_cast<int>(wordCount * 0.85) + 10;  // +10 for overhead
}

float Generator::estimateCost(int inputTokens, int outputTokens,
                             const std::string &model) {
    // Pricing for different models (per 1M tokens)
    // DeepSeek-V3: $0.27 input / $1.10 output (as of 2025)
    // GPT-4: $30 input / $60 output
    // Claude: $3 input / $15 output

    float inputPricePer1M = 0.27f;   // Default to DeepSeek
    float outputPricePer1M = 1.10f;

    if (model.find("gpt-4") != std::string::npos) {
        inputPricePer1M = 30.0f;
        outputPricePer1M = 60.0f;
    } else if (model.find("claude") != std::string::npos) {
        inputPricePer1M = 3.0f;
        outputPricePer1M = 15.0f;
    }

    float inputCost = (inputTokens / 1000000.0f) * inputPricePer1M;
    float outputCost = (outputTokens / 1000000.0f) * outputPricePer1M;

    return inputCost + outputCost;
}
