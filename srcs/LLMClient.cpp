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
    const char *apiKey = std::getenv("LLM_API_KEY");
    if (apiKey == NULL || std::strlen(apiKey) == 0) {
        _lastError = "LLM_API_KEY environment variable not set";
        return false;
    }
    _apiKey = apiKey;

#if USE_ENV_VARS
    const char *apiUrl = std::getenv("LLM_API_URL");
    if (apiUrl == NULL || std::strlen(apiUrl) == 0) {
        _lastError = "LLM_API_URL environment variable not set";
        return false;
    }
    _apiUrl = apiUrl;

    const char *model = std::getenv("LLM_MODEL");
    if (model == NULL || std::strlen(model) == 0) {
        _lastError = "LLM_MODEL environment variable not set";
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
                "format with exactly two fields: correct (boolean) "
                "and hint (string). Provide hints, not solutions.\"},"
                << "{\"role\":\"user\",\"content\":"
             << escapedPrompt << "}]"
#if USE_RESPONSE_FORMAT
             << ",\"response_format\":{\"type\":\"json_object\"}"
#endif
             << ",\"temperature\":0.3"
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

bool LLMClient::complete(const std::string &prompt, std::string &response) {
    std::string escapedPrompt = escapeJson(prompt);

    std::stringstream jsonBody;
    jsonBody << "{"
             << "\"model\":\"" << _model << "\","
             << "\"messages\":[{\"role\":\"system\",\"content\":\"You "
                "are a 42 school exercise generator. Generate complete, "
                "compilable C exercises following the specifications exactly. "
                "Always respond with valid JSON only, no additional text.\"},"
                << "{\"role\":\"user\",\"content\":"
             << escapedPrompt << "}]"
#if USE_RESPONSE_FORMAT
             << ",\"response_format\":{\"type\":\"json_object\"}"
#endif
             << ",\"temperature\":0.7"
             << "}";

    std::string postFields = jsonBody.str();
    std::string apiResponse;

    std::vector<std::string> headers;
    headers.push_back("Content-Type: application/json");
    headers.push_back("Authorization: Bearer " + _apiKey);

    if (!_httpClient.post(_apiUrl, headers, postFields, apiResponse)) {
        _lastError = _httpClient.getLastError();
        return false;
    }

    // Extract the content from the API response
    size_t contentPos = apiResponse.find("\"content\":");
    if (contentPos != std::string::npos) {
        size_t start = apiResponse.find('"', contentPos + 10);
        if (start != std::string::npos) {
            start++;
            size_t end = start;
            while (end < apiResponse.length()) {
                if (apiResponse[end] == '"' && apiResponse[end - 1] != '\\') {
                    break;
                }
                end++;
            }
            if (end < apiResponse.length()) {
                response = apiResponse.substr(start, end - start);

                // Remove markdown code blocks if present
                size_t codeBlockStart = response.find("```json");
                if (codeBlockStart != std::string::npos) {
                    codeBlockStart += 7;
                    size_t codeBlockEnd = response.find("```", codeBlockStart);
                    if (codeBlockEnd != std::string::npos) {
                        response = response.substr(codeBlockStart, codeBlockEnd - codeBlockStart);
                        // Trim whitespace
                        size_t first = response.find_first_not_of(" \t\n\r");
                        size_t last = response.find_last_not_of(" \t\n\r");
                        if (first != std::string::npos && last != std::string::npos) {
                            response = response.substr(first, last - first + 1);
                        }
                    }
                }

                return true;
            }
        }
    }

    _lastError = "Invalid API response format: missing content";
    return false;
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
            // Find the closing quote, handling escaped quotes properly
            size_t end = start;
            while (end < json.length()) {
                if (json[end] == '"' && json[end - 1] != '\\') {
                    break;
                }
                end++;
            }
            if (end < json.length()) {
                content = json.substr(start, end - start);
            }
        }
    }

    if (content.empty()) {
        content = json;
    }

    // First, unescape the content (convert \n to actual newlines, etc.)
    std::string unescaped;
    for (size_t i = 0; i < content.length(); ++i) {
        if (content[i] == '\\' && i + 1 < content.length()) {
            switch (content[i + 1]) {
                case 'n': unescaped += '\n'; i++; break;
                case 't': unescaped += '\t'; i++; break;
                case 'r': unescaped += '\r'; i++; break;
                case '\\': unescaped += '\\'; i++; break;
                case '"': unescaped += '"'; i++; break;
                default: unescaped += content[i]; break;
            }
        } else {
            unescaped += content[i];
        }
    }
    content = unescaped;

    // Handle markdown code blocks (some models wrap JSON in ```json ... ```)
    size_t codeBlockStart = content.find("```json");
    if (codeBlockStart != std::string::npos) {
        codeBlockStart += 7; // Skip "```json"
        size_t codeBlockEnd = content.find("```", codeBlockStart);
        if (codeBlockEnd != std::string::npos) {
            // Extract content between code block markers
            content = content.substr(codeBlockStart, codeBlockEnd - codeBlockStart);
            // Trim whitespace
            size_t first = content.find_first_not_of(" \t\n\r");
            size_t last = content.find_last_not_of(" \t\n\r");
            if (first != std::string::npos && last != std::string::npos) {
                content = content.substr(first, last - first + 1);
            }
        }
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
