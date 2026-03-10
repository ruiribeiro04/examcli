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

void Result::setCorrectionResult(bool passed, const std::string &hint) {
    _passed = passed;
    _message = hint;
}

void Result::show() const {
    if (_passed) {
        std::cout << "✓ Exercise passed\n";
    } else {
        std::cout << "✗ Exercise failed\n";
    }

    if (!_message.empty()) {
        std::cout << "\nHint:\n";

        size_t pos = 0;
        const size_t maxLineLength = 80;

        while (pos < _message.length()) {
            size_t newlinePos = _message.find('\n', pos);
            size_t lineEnd;

            if (newlinePos == std::string::npos) {
                lineEnd = _message.length();
            } else {
                lineEnd = newlinePos;
            }

            std::string line = _message.substr(pos, lineEnd - pos);

            while (line.length() > maxLineLength) {
                size_t breakPos = line.rfind(' ', maxLineLength);
                if (breakPos == std::string::npos || breakPos == 0) {
                    breakPos = maxLineLength;
                }
                std::cout << line.substr(0, breakPos) << "\n";
                line = line.substr(breakPos);
                while (!line.empty() && line[0] == ' ') {
                    line = line.substr(1);
                }
            }
            std::cout << line << "\n";

            if (newlinePos == std::string::npos) {
                break;
            }
            pos = newlinePos + 1;
        }
    }
}

int Result::getExitCode() const {
    return _passed ? 0 : 1;
}
