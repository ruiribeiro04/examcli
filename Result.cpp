#include "Result.hpp"

Result::Result () :_message(), _passed() {
    std::cout << "Default constructor called\n";
}

Result::Result(const std::string &message, bool passed)
    : _message(message), _passed(passed) {
    std::cout << "Result constructor called\n";
}

Result::Result(const Result &other)
    : _message(other._message), _passed(other._passed) {
    std::cout << "Copy constructor called\n";
}

Result &Result::operator=(const Result &other) {
    if (this != &other) {
        _message = other._message;
        _passed = other._passed;
    }
    std::cout << "Copy assignment operator called\n";
    return *this;
}

Result::~Result() {
    std::cout << "Destructor called\n";
}

void Result::show() const {
    std::cout << "Result: " << _message
              << " | passed = " << (_passed ? "true" : "false")
              << "\n";
}
