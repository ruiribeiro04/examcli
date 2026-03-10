#include "HttpClient.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

HttpClient::HttpClient() : _timeoutSeconds(30), _lastError() {
}

HttpClient::HttpClient(const HttpClient &other)
    : _timeoutSeconds(other._timeoutSeconds), _lastError(other._lastError) {
}

HttpClient &HttpClient::operator=(const HttpClient &other) {
    if (this != &other) {
        _timeoutSeconds = other._timeoutSeconds;
        _lastError = other._lastError;
    }
    return *this;
}

HttpClient::~HttpClient() {
}

bool HttpClient::post(const std::string &url,
                      const std::vector<std::string> &headers,
                      const std::string &body,
                      std::string &response) {
    std::string bodyFile = createTempFile(body);
    if (bodyFile.empty()) {
        _lastError = "Failed to create temporary file for request body";
        return false;
    }

    bool success = executeCurl(url, headers, bodyFile, response);
    deleteTempFile(bodyFile);

    return success;
}

void HttpClient::setTimeout(int seconds) {
    _timeoutSeconds = seconds;
}

const std::string &HttpClient::getLastError() const {
    return _lastError;
}

std::string HttpClient::createTempFile(const std::string &data) {
    char tmpFile[] = "/tmp/examcli_XXXXXX";
    int fd = mkstemp(tmpFile);
    if (fd == -1) {
        return "";
    }
    close(fd);

    std::ofstream ofs(tmpFile);
    if (!ofs.is_open()) {
        remove(tmpFile);
        return "";
    }

    ofs << data;
    ofs.close();

    return std::string(tmpFile);
}

bool HttpClient::executeCurl(const std::string &url,
                             const std::vector<std::string> &headers,
                             const std::string &bodyFile,
                             std::string &response) {
    std::stringstream cmd;
    cmd << "curl -s -X POST";
    cmd << " --max-time " << _timeoutSeconds;

    for (size_t i = 0; i < headers.size(); ++i) {
        cmd << " -H " << escapeShellArg(headers[i]);
    }

    cmd << " -d @" << bodyFile;
    cmd << " " << escapeShellArg(url);

    std::string command = cmd.str();
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == NULL) {
        _lastError = "Failed to execute curl command";
        return false;
    }

    response.clear();
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        response.append(buffer);
    }

    int exitCode = pclose(pipe);
    if (exitCode != 0) {
        std::stringstream errorMsg;
        errorMsg << "curl command failed with exit code: " << exitCode;
        _lastError = errorMsg.str();
        return false;
    }

    return true;
}

void HttpClient::deleteTempFile(const std::string &filename) {
    if (!filename.empty()) {
        remove(filename.c_str());
    }
}

std::string HttpClient::escapeShellArg(const std::string &arg) {
    std::stringstream escaped;
    escaped << "'";

    for (size_t i = 0; i < arg.length(); ++i) {
        char c = arg[i];
        if (c == '\'') {
            escaped << "'\\''";
        } else {
            escaped << c;
        }
    }

    escaped << "'";
    return escaped.str();
}
