#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP

#include <string>
#include <iostream>

class Submission {
private:
    std::string* _filePath;
    std::string* _subject;

public:
    Submission();
    Submission(const std::string &file, const std::string &subject);
    Submission(const Submission &other);
    Submission &operator=(const Submission &other);
    ~Submission();

    void send() const;
    void show() const;
};

#endif
