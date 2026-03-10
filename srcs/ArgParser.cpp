#include "ArgParser.hpp"
#include <iostream>
#include <fstream>

ArgParser::ArgParser() : _rank(), _level(), _subject(), _sourceFile(), _showHelp(false),
    _listRanks(false), _listLevels(false), _listSubjects(false) {
}

ArgParser::ArgParser(const ArgParser &other)
    : _rank(other._rank), _level(other._level),
      _subject(other._subject), _sourceFile(other._sourceFile), _showHelp(other._showHelp),
      _listRanks(other._listRanks), _listLevels(other._listLevels), _listSubjects(other._listSubjects) {
}

ArgParser &ArgParser::operator=(const ArgParser &other) {
    if (this != &other) {
        _rank = other._rank;
        _level = other._level;
        _subject = other._subject;
        _sourceFile = other._sourceFile;
        _showHelp = other._showHelp;
        _listRanks = other._listRanks;
        _listLevels = other._listLevels;
        _listSubjects = other._listSubjects;
    }
    return *this;
}

ArgParser::~ArgParser() {
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
        } else if (arg == "--rank" || arg == "-r") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --rank requires an argument\n";
                showUsage();
                return false;
            }
            _rank = argv[++i];
        } else if (arg == "--level" || arg == "-l") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --level requires an argument\n";
                showUsage();
                return false;
            }
            _level = argv[++i];
        } else if (arg == "--subject" || arg == "-s") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --subject requires an argument\n";
                showUsage();
                return false;
            }
            _subject = argv[++i];
        } else if (arg == "--list-ranks" || arg == "--lr") {
            _listRanks = true;
        } else if (arg == "--list-levels" || arg == "--ll") {
            _listLevels = true;
        } else if (arg == "--list-subjects" || arg == "--ls") {
            _listSubjects = true;
        } else if (arg[0] == '-') {
            std::cerr << "Error: Unknown argument: " << arg << "\n";
            showUsage();
            return false;
        } else {
            _sourceFile = arg;
        }
    }

    bool isListMode = _listRanks || _listLevels || _listSubjects;
    if (!_showHelp && !isListMode && _sourceFile.empty()) {
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
              << "  -r, --rank <name>       Rank name (e.g., rank02)\n"
              << "  -l, --level <name>      Level name (e.g., level0)\n"
              << "  -s, --subject <name>    Subject/exercise name (e.g., fizzbuzz)\n"
              << "  -h, --help              Show this help message\n"
              << "  --list-ranks, --lr      List available ranks\n"
              << "  --list-levels, --ll     List levels for a rank (requires -r)\n"
              << "  --list-subjects, --ls   List subjects for rank/level (requires -r -l)\n"
              << "\n"
              << "Examples:\n"
              << "  ./examcli -r rank02 -l level0 -s fizzbuzz solution.c\n"
              << "  ./examcli --list-ranks\n"
              << "  ./examcli -r rank04 --list-levels\n"
              << "  ./examcli -r rank04 -l level1 --list-subjects\n";
}

bool ArgParser::hasRank() const {
    return !_rank.empty();
}

bool ArgParser::hasLevel() const {
    return !_level.empty();
}

bool ArgParser::hasSubject() const {
    return !_subject.empty();
}

const std::string &ArgParser::getRank() const {
    return _rank;
}

const std::string &ArgParser::getLevel() const {
    return _level;
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

bool ArgParser::shouldListRanks() const {
    return _listRanks;
}

bool ArgParser::shouldListLevels() const {
    return _listLevels;
}

bool ArgParser::shouldListSubjects() const {
    return _listSubjects;
}

bool ArgParser::fileExists(const std::string &path) const {
    std::ifstream file(path.c_str());
    return file.good();
}
