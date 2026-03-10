#ifndef LLMCLIENT_HPP
#define LLMCLIENT_HPP

#include <string>

class LLMClient {
private:
    std::string _apiKey;
    std::string _apiUrl;
    std::string _model;
    std::string _lastError;
    static const int TIMEOUT_SECONDS = 30;

public:
    LLMClient();
    LLMClient(const LLMClient &other);
    LLMClient &operator=(const LLMClient &other);
    ~LLMClient();

    bool initialize();
    bool correct(const std::string &subject, const std::string &code,
                 bool &correct, std::string &hint);
    const std::string &getLastError() const;

private:
    std::string buildPrompt(const std::string &subject,
                            const std::string &code) const;
    std::string escapeJson(const std::string &input) const;
    bool parseResponse(const std::string &json, bool &correct,
                       std::string &hint);
    size_t findJsonValue(const std::string &json, const std::string &key,
                         std::string &value) const;
};

#endif
