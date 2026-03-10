#include "ArgParser.hpp"
#include <iostream>
#include <fstream>

ArgParser::ArgParser() : _exam(), _subject(), _sourceFile(), _showHelp(false) {
    std::cout << "ArgParser default constructor called\n";
}

ArgParser::ArgParser(const ArgParser &other)
    : _exam(other._exam), _subject(other._subject),
      _sourceFile(other._sourceFile), _showHelp(other._showHelp) {
    std::cout << "ArgParser copy constructor called\n";
}

ArgParser &ArgParser::operator=(const ArgParser &other) {
    std::cout << "ArgParser copy assignment operator called\n";
    if (this != &other) {
        _exam = other._exam;
        _subject = other._subject;
        _sourceFile = other._sourceFile;
        _showHelp = other._showHelp;
    }
    return *this;
}

ArgParser::~ArgParser() {
    std::cout << "ArgParser destructor called\n";
}

bool ArgParser::parse(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Error: No source file provided\n";
        showUsage();
        return false;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            _showHelp = true;
            showUsage();
            return true;
        } else if (arg == "--exam") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --exam requires an argument\n";
                showUsage();
                return false;
            }
            _exam = argv[++i];
        } else if (arg == "--subject") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --subject requires an argument\n";
                showUsage();
                return false;
            }
            _subject = argv[++i];
        } else if (arg[0] == '-') {
            std::cerr << "Error: Unknown argument: " << arg << "\n";
            showUsage();
            return false;
        } else {
            _sourceFile = arg;
        }
    }

    if (!_showHelp && _sourceFile.empty()) {
        std::cerr << "Error: No source file provided\n";
        showUsage();
        return false;
    }

    if (!_sourceFile.empty() && !fileExists(_sourceFile)) {
        std::cerr << "Error: File not found: " << _sourceFile << "\n";
        return false;
    }

    return true;
}

void ArgParser::showUsage() const {
    std::cout << "Usage: ./examcli [options] <source_file>\n"
              << "\n"
              << "Options:\n"
              << "  --exam <name>      Exam name (e.g., exam02)\n"
              << "  --subject <name>   Subject/exercise name (e.g., ex01)\n"
              << "  -h, --help         Show this help message\n"
              << "\n"
              << "Example:\n"
              << "  ./examcli --exam exam02 --subject ex01 solution.cpp\n";
}

bool ArgParser::hasExam() const {
    return !_exam.empty();
}

bool ArgParser::hasSubject() const {
    return !_subject.empty();
}

const std::string &ArgParser::getExam() const {
    return _exam;
}

const std::string &ArgParser::getSubject() const {
    return _subject;
}

const std::string &ArgParser::getSourceFile() const {
    return _sourceFile;
}

bool ArgParser::shouldShowHelp() const {
    return _showHelp;
}

bool ArgParser::fileExists(const std::string &path) const {
    std::ifstream file(path.c_str());
    return file.good();
}
