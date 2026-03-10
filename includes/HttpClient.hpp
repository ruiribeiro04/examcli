#ifndef HTTPCLIENT_HPP
#define HTTPCLIENT_HPP

#include <string>
#include <vector>
#include <unistd.h>

class HttpClient {
private:
    int _timeoutSeconds;
    std::string _lastError;

public:
    HttpClient();
    HttpClient(const HttpClient &other);
    HttpClient &operator=(const HttpClient &other);
    ~HttpClient();

    bool post(const std::string &url,
              const std::vector<std::string> &headers,
              const std::string &body,
              std::string &response);

    void setTimeout(int seconds);
    const std::string &getLastError() const;

private:
    std::string createTempFile(const std::string &data);
    bool executeCurl(const std::string &url,
                     const std::vector<std::string> &headers,
                     const std::string &bodyFile,
                     std::string &response);
    void deleteTempFile(const std::string &filename);
    std::string escapeShellArg(const std::string &arg);
};

#endif
