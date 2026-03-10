#include "LLMClient.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <curl/curl.h>
#include <cstdlib>

LLMClient::LLMClient() : _apiKey(), _apiUrl(), _model(), _lastError() {
    std::cout << "LLMClient default constructor called\n";
}

LLMClient::LLMClient(const LLMClient &other)
    : _apiKey(other._apiKey), _apiUrl(other._apiUrl),
      _model(other._model), _lastError(other._lastError) {
    std::cout << "LLMClient copy constructor called\n";
}

LLMClient &LLMClient::operator=(const LLMClient &other) {
    std::cout << "LLMClient copy assignment operator called\n";
    if (this != &other) {
        _apiKey = other._apiKey;
        _apiUrl = other._apiUrl;
        _model = other._model;
        _lastError = other._lastError;
    }
    return *this;
}

LLMClient::~LLMClient() {
    std::cout << "LLMClient destructor called\n";
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

static size_t writeCallback(void *contents, size_t size, size_t nmemb,
                            std::string *userp) {
    userp->append((char *)contents, size * nmemb);
    return size * nmemb;
}

bool LLMClient::correct(const std::string &subject, const std::string &code,
                        bool &correct, std::string &hint) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        _lastError = "Failed to initialize CURL";
        return false;
    }

    std::string prompt = buildPrompt(subject, code);
    std::string escapedPrompt = escapeJson(prompt);

    std::stringstream jsonBody;
    jsonBody << "{"
             << "\"model\":\"" << _model << "\","
             << "\"messages\":[{\"role\":\"system\",\"content\":\"You "
                "are a 42 school code evaluator. Respond only in JSON "
                "format with exactly two fields: 'correct' (boolean) "
                "and 'hint' (string). Provide hints, not solutions.\"},"
                "{\"role\":\"user\",\"content\":"
             << escapedPrompt << "}],"
             << "\"response_format\":{\"type\":\"json_object\"},"
             << "\"temperature\":0.3"
             << "}";

    std::string postFields = jsonBody.str();
    std::string response;

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string authHeader = "Authorization: Bearer " + _apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, _apiUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        if (res == CURLE_OPERATION_TIMEDOUT) {
            _lastError = "API request timed out";
        } else {
            _lastError = std::string("CURL error: ") + curl_easy_strerror(res);
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return false;
    }

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (httpCode != 200) {
        std::stringstream errorMsg;
        errorMsg << "API request failed: HTTP " << httpCode;
        _lastError = errorMsg.str();
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
