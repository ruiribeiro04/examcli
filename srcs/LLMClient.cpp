#include "LLMClient.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>

LLMClient::LLMClient() : _apiKey(), _apiUrl(), _model(), _lastError(), _httpClient() {
    _httpClient.setTimeout(TIMEOUT_SECONDS);
}

LLMClient::LLMClient(const LLMClient &other)
    : _apiKey(other._apiKey), _apiUrl(other._apiUrl),
      _model(other._model), _lastError(other._lastError),
      _httpClient(other._httpClient) {
}

LLMClient &LLMClient::operator=(const LLMClient &other) {
    if (this != &other) {
        _apiKey = other._apiKey;
        _apiUrl = other._apiUrl;
        _model = other._model;
        _lastError = other._lastError;
        _httpClient = other._httpClient;
    }
    return *this;
}

LLMClient::~LLMClient() {
}

bool LLMClient::initialize() {
    const char *apiKey = std::getenv("OPENAI_API_KEY");
    if (apiKey == NULL || std::strlen(apiKey) == 0) {
        _lastError = "OPENAI_API_KEY environment variable not set";
        return false;
    }
    _apiKey = apiKey;

#if USE_ENV_VARS
    const char *apiUrl = std::getenv("OPENAI_API_URL");
    if (apiUrl == NULL || std::strlen(apiUrl) == 0) {
        _lastError = "OPENAI_API_URL environment variable not set";
        return false;
    }
    _apiUrl = apiUrl;

    const char *model = std::getenv("OPENAI_MODEL");
    if (model == NULL || std::strlen(model) == 0) {
        _lastError = "OPENAI_MODEL environment variable not set";
        return false;
    }
    _model = model;
#else
    _apiUrl = DEFAULT_API_URL;
    _model = DEFAULT_MODEL;
#endif

    return true;
}

bool LLMClient::correct(const std::string &subject, const std::string &code,
                        bool &correct, std::string &hint) {
    std::string prompt = buildPrompt(subject, code);
    std::string escapedPrompt = escapeJson(prompt);

    std::stringstream jsonBody;
    jsonBody << "{"
             << "\"model\":\"" << _model << "\","
             << "\"messages\":[{\"role\":\"system\",\"content\":\"You "
                "are a 42 school code evaluator. Respond only in JSON "
                "format with exactly two fields: 'correct' (boolean) "
                "and 'hint' (string). Provide hints, not solutions.\"},"
                << "{\"role\":\"user\",\"content\":"
             << escapedPrompt << "}],"
             << "\"response_format\":{\"type\":\"json_object\"},"
             << "\"temperature\":0.3"
             << "}";

    std::string postFields = jsonBody.str();
    std::string response;

    std::vector<std::string> headers;
    headers.push_back("Content-Type: application/json");
    headers.push_back("Authorization: Bearer " + _apiKey);

    if (!_httpClient.post(_apiUrl, headers, postFields, response)) {
        _lastError = _httpClient.getLastError();
        return false;
    }

    if (!parseResponse(response, correct, hint)) {
        return false;
    }

    return true;
}

const std::string &LLMClient::getLastError() const {
    return _lastError;
}

std::string LLMClient::buildPrompt(const std::string &subject,
                                   const std::string &code) const {
    std::stringstream prompt;
    prompt << "Subject:\n"
           << subject << "\n\n"
           << "Student Code:\n"
           << code << "\n\n"
           << "Evaluate this 42 C++98 exercise submission. Check:\n"
           << "1. Code compiles with -std=c++98 -Wall -Wextra -Werror\n"
           << "2. Follows Orthodox Canonical Form (default constructor, copy "
              "constructor, copy assignment, destructor)\n"
           << "3. Proper header guards and naming conventions\n"
           << "4. Functionality matches subject requirements\n\n"
           << "Respond in JSON format with:\n"
           << "- correct: true if exercise passes, false otherwise\n"
           << "- hint: brief guidance on what to check/improve (NOT the "
              "solution)";
    return prompt.str();
}

std::string LLMClient::escapeJson(const std::string &input) const {
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

bool LLMClient::parseResponse(const std::string &json, bool &correct,
                              std::string &hint) {
    std::string content;

    size_t contentPos = json.find("\"content\":");
    if (contentPos != std::string::npos) {
        size_t start = json.find('"', contentPos + 10);
        if (start != std::string::npos) {
            start++;
            size_t end = json.find('"', start);
            while (end != std::string::npos && json[end - 1] == '\\') {
                end = json.find('"', end + 1);
            }
            if (end != std::string::npos) {
                content = json.substr(start, end - start);
            }
        }
    }

    if (content.empty()) {
        content = json;
    }

    std::string correctStr;
    if (findJsonValue(content, "correct", correctStr)) {
        correct = (correctStr == "true");
    } else {
        _lastError = "Invalid API response format: missing 'correct' field";
        return false;
    }

    if (!findJsonValue(content, "hint", hint)) {
        _lastError = "Invalid API response format: missing 'hint' field";
        return false;
    }

    size_t pos = 0;
    while ((pos = hint.find("\\n", pos)) != std::string::npos) {
        hint.replace(pos, 2, "\n");
        pos += 1;
    }

    return true;
}

size_t LLMClient::findJsonValue(const std::string &json,
                                const std::string &key,
                                std::string &value) const {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);

    if (keyPos == std::string::npos) {
        return std::string::npos;
    }

    size_t colonPos = json.find(':', keyPos + searchKey.length());
    if (colonPos == std::string::npos) {
        return std::string::npos;
    }

    size_t valueStart = json.find_first_not_of(" \t\n\r", colonPos + 1);
    if (valueStart == std::string::npos) {
        return std::string::npos;
    }

    if (json[valueStart] == '"') {
        valueStart++;
        size_t valueEnd = valueStart;
        while (valueEnd < json.length()) {
            if (json[valueEnd] == '"' && json[valueEnd - 1] != '\\') {
                break;
            }
            valueEnd++;
        }
        value = json.substr(valueStart, valueEnd - valueStart);
        return valueStart;
    } else {
        size_t valueEnd = json.find_first_of(",}\n", valueStart);
        if (valueEnd == std::string::npos) {
            valueEnd = json.length();
        }
        value = json.substr(valueStart, valueEnd - valueStart);
        size_t trimEnd = value.find_last_not_of(" \t");
        if (trimEnd != std::string::npos) {
            value = value.substr(0, trimEnd + 1);
        }
        return valueStart;
    }
}
