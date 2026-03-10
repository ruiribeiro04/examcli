#include "Submission.hpp"

Submission::Submission()
    : _filePath(), _subject() {
}

Submission::Submission(const std::string &file, const std::string &subject)
    : _filePath(file), _subject(subject) {
}

Submission::Submission(const Submission &other)
    : _filePath(other._filePath), _subject(other._subject) {
}

Submission &Submission::operator=(const Submission &other) {
    if (this != &other) {
        _filePath = other._filePath;
        _subject = other._subject;
    }
    return *this;
}

Submission::~Submission() {
}

void Submission::send() const {
    std::cout << "Sending file: " << _filePath
              << " subject: " << _subject << "\n";
}

void Submission::show() const {
    std::cout << "Submission file: " << _filePath
              << " subject: " << _subject << "\n";
}