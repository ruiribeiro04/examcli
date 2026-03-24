#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <string>
#include <vector>
#include <map>
#include "LLMClient.hpp"

struct TestCase {
    std::string description;
    std::vector<std::string> input;
    std::string expected;
    std::string explanation;
};

struct ExerciseData {
    std::string exercise_name;
    std::string exercise_type;
    std::string subject;
    std::string solution_code;
    std::vector<TestCase> test_cases;

    bool isValid() const;
};

class Generator {
private:
    LLMClient _llmClient;
    std::string _rank;
    std::string _level;
    std::string _exerciseType;
    std::map<std::string, std::vector<std::string> > _constraints;

    std::string buildGenerationPrompt(const std::string &userPrompt) const;
    std::string buildRefinementPrompt(const std::string &error,
                                     const std::string &stage) const;
    bool parseExerciseResponse(const std::string &json,
                               ExerciseData &exercise);
    bool loadConstraints(const std::string &constraintsFile);
    std::string getConstraintsForRank(const std::string &rank) const;
    std::string escapeJson(const std::string &input) const;
    std::string unescapeJson(const std::string &input) const;

public:
    Generator();
    Generator(const Generator &other);
    Generator &operator=(const Generator &other);
    ~Generator();

    bool initialize(const std::string &rank = "rank02",
                   const std::string &level = "level0",
                   const std::string &type = "function");
    bool generate(const std::string &prompt, ExerciseData &exercise,
                 int maxRetries = 3);
    const std::string &getLastError() const;
    void setRank(const std::string &rank);
    void setLevel(const std::string &level);
    void setExerciseType(const std::string &type);

    // Cost estimation (task 11.5)
    static int estimateTokenCount(const std::string &text);
    static float estimateCost(int inputTokens, int outputTokens,
                             const std::string &model = "deepseek-chat");

private:
    std::string _lastError;
};

#endif
