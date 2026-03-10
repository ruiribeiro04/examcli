#ifndef RESULT_HPP
#define RESULT_HPP

#include <string>
#include <iostream>

class Result {
private:
    std::string _message;
    bool _passed;

public:
    Result();
    Result(const std::string &message, bool passed);
    Result(const Result &other);
    Result &operator=(const Result &other);
    ~Result();

    void setCorrectionResult(bool passed, const std::string &hint);
    void show() const;
    int getExitCode() const;
};

#endif