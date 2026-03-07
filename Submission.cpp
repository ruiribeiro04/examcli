#include "Submission.hpp"

Submission::Submission()
    : _filePath(), _subject() {
    std::cout << "Default Submission constructor called\n";
}

Submission::Submission(const std::string &file, const std::string &subject)
    : _filePath(file), _subject(subject) {
    std::cout << "Submission constructor called\n";
}

Submission::Submission(const Submission &other)
    : _filePath(other._filePath), _subject(other._subject) {
    std::cout << "Submission copy constructor called\n";
}

Submission &Submission::operator=(const Submission &other) {
    std::cout << "Submission copy assignment operator called\n";
    if (this != &other) {
        _filePath = other._filePath;
        _subject = other._subject;
    }
    return *this;
}

Submission::~Submission() {
    std::cout << "Submission destructor called\n";
}

void Submission::send() const {
    std::cout << "Sending file: " << _filePath
              << " subject: " << _subject << "\n";
}

void Submission::show() const {
    std::cout << "Submission file: " << _filePath
              << " subject: " << _subject << "\n";
}