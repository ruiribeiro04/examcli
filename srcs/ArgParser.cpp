#include "ArgParser.hpp"
#include <iostream>
#include <fstream>

ArgParser::ArgParser()
    : _command(CMD_PRACTICE), _rank(), _level(), _subject(), _sourceFile(),
      _prompt(), _outputPath(), _importFile(), _ratingScore(), _exerciseName(),
      _showHelp(false), _listRanks(false), _listLevels(false), _listSubjects(false),
      _listGenerated(false), _listShareable(false), _showAll(false), _metadataOnly(false) {
}

ArgParser::ArgParser(const ArgParser &other)
    : _command(other._command), _rank(other._rank), _level(other._level),
      _subject(other._subject), _sourceFile(other._sourceFile), _prompt(other._prompt),
      _outputPath(other._outputPath), _importFile(other._importFile),
      _ratingScore(other._ratingScore), _exerciseName(other._exerciseName),
      _showHelp(other._showHelp), _listRanks(other._listRanks), _listLevels(other._listLevels),
      _listSubjects(other._listSubjects), _listGenerated(other._listGenerated),
      _listShareable(other._listShareable), _showAll(other._showAll),
      _metadataOnly(other._metadataOnly) {
}

ArgParser &ArgParser::operator=(const ArgParser &other) {
    if (this != &other) {
        _command = other._command;
        _rank = other._rank;
        _level = other._level;
        _subject = other._subject;
        _sourceFile = other._sourceFile;
        _prompt = other._prompt;
        _outputPath = other._outputPath;
        _importFile = other._importFile;
        _ratingScore = other._ratingScore;
        _exerciseName = other._exerciseName;
        _showHelp = other._showHelp;
        _listRanks = other._listRanks;
        _listLevels = other._listLevels;
        _listSubjects = other._listSubjects;
        _listGenerated = other._listGenerated;
        _listShareable = other._listShareable;
        _showAll = other._showAll;
        _metadataOnly = other._metadataOnly;
    }
    return *this;
}

ArgParser::~ArgParser() {
}

bool ArgParser::parse(int argc, char **argv) {
    if (argc < 2) {
        _showAll = true;
        return true;
    }

    // First pass: check for command
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "generate") {
            _command = CMD_GENERATE;
        } else if (arg == "share") {
            _command = CMD_SHARE;
        } else if (arg == "import") {
            _command = CMD_IMPORT;
        } else if (arg == "list") {
            _command = CMD_LIST;
        } else if (arg == "rate") {
            _command = CMD_RATE;
        } else if (arg == "info") {
            _command = CMD_INFO;
        }
    }

    // Second pass: parse arguments
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
        } else if (arg == "--type") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --type requires an argument\n";
                showUsage();
                return false;
            }
            // Store type as part of prompt for now
            _prompt += " type:" + std::string(argv[++i]);
        } else if (arg == "--list-ranks" || arg == "--lr") {
            _listRanks = true;
        } else if (arg == "--list-levels" || arg == "--ll") {
            _listLevels = true;
        } else if (arg == "--list-subjects" || arg == "--ls") {
            _listSubjects = true;
        } else if (arg == "--generated") {
            _listGenerated = true;
        } else if (arg == "--shareable") {
            _listShareable = true;
        } else if (arg == "-o") {
            if (i + 1 >= argc) {
                std::cerr << "Error: -o requires an argument\n";
                showUsage();
                return false;
            }
            _outputPath = argv[++i];
        } else if (arg == "--metadata-only") {
            _metadataOnly = true;
        } else if (arg == "generate" || arg == "share" || arg == "import" ||
                   arg == "list" || arg == "rate" || arg == "info") {
            // Skip commands - already processed
            continue;
        } else if (arg[0] == '-') {
            std::cerr << "Error: Unknown argument: " << arg << "\n";
            showUsage();
            return false;
        } else {
            // Positional arguments
            if (_command == CMD_GENERATE) {
                if (_prompt.empty()) {
                    _prompt = arg;
                }
            } else if (_command == CMD_SHARE) {
                if (_exerciseName.empty()) {
                    _exerciseName = arg;
                }
            } else if (_command == CMD_IMPORT) {
                if (_importFile.empty()) {
                    _importFile = arg;
                }
            } else if (_command == CMD_RATE) {
                if (_exerciseName.empty()) {
                    _exerciseName = arg;
                } else if (_ratingScore.empty()) {
                    _ratingScore = arg;
                }
            } else if (_command == CMD_INFO) {
                if (_exerciseName.empty()) {
                    _exerciseName = arg;
                }
            } else {
                _sourceFile = arg;
            }
        }
    }

    // Validation based on command
    bool isListMode = _listRanks || _listLevels || _listSubjects || _listGenerated || _listShareable;

    if (_command == CMD_GENERATE) {
        if (_prompt.empty()) {
            std::cerr << "Error: generate command requires a prompt\n";
            showUsage();
            return false;
        }
    } else if (_command == CMD_SHARE) {
        if (_exerciseName.empty()) {
            std::cerr << "Error: share command requires an exercise name\n";
            showUsage();
            return false;
        }
    } else if (_command == CMD_IMPORT) {
        if (_importFile.empty()) {
            std::cerr << "Error: import command requires a file path\n";
            showUsage();
            return false;
        }
    } else if (_command == CMD_RATE) {
        if (_exerciseName.empty() || _ratingScore.empty()) {
            std::cerr << "Error: rate command requires exercise name and score\n";
            showUsage();
            return false;
        }
    } else if (_command == CMD_INFO) {
        if (_exerciseName.empty()) {
            std::cerr << "Error: info command requires an exercise name\n";
            showUsage();
            return false;
        }
    } else if (_command == CMD_PRACTICE) {
        if (!_showHelp && !isListMode && _sourceFile.empty()) {
            std::cerr << "Error: No source file provided\n";
            showUsage();
            return false;
        }
        if (!_sourceFile.empty() && !fileExists(_sourceFile)) {
            std::cerr << "Error: File not found: " << _sourceFile << "\n";
            return false;
        }
    }

    return true;
}

void ArgParser::showUsage() const {
    std::cout << "Usage: ./examcli <command> [options] [arguments]\n"
              << "\n"
              << "Commands:\n"
              << "  generate <prompt>        Generate a new exercise from prompt\n"
              << "  share <exercise>         Export exercise as tar.gz archive\n"
              << "  import <file.tar.gz>     Import exercise from archive\n"
              << "  list [options]           List available exercises\n"
              << "  rate <exercise> <score>  Rate an exercise (1-5)\n"
              << "  info <exercise>          Show exercise details\n"
              << "  [practice]               Practice mode (default, provide source file)\n"
              << "\n"
              << "Practice Mode Options:\n"
              << "  -r, --rank <name>       Rank name (e.g., rank02)\n"
              << "  -l, --level <name>      Level name (e.g., level0)\n"
              << "  -s, --subject <name>    Subject/exercise name (e.g., fizzbuzz)\n"
              << "  -h, --help              Show this help message\n"
              << "\n"
              << "List Options:\n"
              << "  --list-ranks, --lr      List available ranks\n"
              << "  --list-levels, --ll     List levels for a rank (requires -r)\n"
              << "  --list-subjects, --ls   List subjects for rank/level (requires -r -l)\n"
              << "  --generated             List generated exercises\n"
              << "  --shareable             List shareable exercises\n"
              << "\n"
              << "Generate Options:\n"
              << "  --rank <name>           Target rank (default: rank02)\n"
              << "  --level <name>          Target level (default: level0)\n"
              << "  --type <function|prog>  Exercise type\n"
              << "\n"
              << "Share Options:\n"
              << "  -o <path>               Output file path\n"
              << "  --metadata-only         Export metadata only\n"
              << "\n"
              << "Import Options:\n"
              << "  --rank <name>           Override target rank\n"
              << "\n"
              << "Examples:\n"
              << "  ./examcli -r rank02 -l level0 -s fizzbuzz solution.c\n"
              << "  ./examcli generate \"linked list with insert\" --rank rank03\n"
              << "  ./examcli share my_linked_list -o /tmp/exercise.tar.gz\n"
              << "  ./examcli import exercise.tar.gz --rank rank04\n"
              << "  ./examcli list --generated\n"
              << "  ./examcli rate my_exercise 5\n"
              << "  ./examcli info my_exercise\n";
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

bool ArgParser::shouldShowAll() const {
    return _showAll;
}

bool ArgParser::fileExists(const std::string &path) const {
    std::ifstream file(path.c_str());
    return file.good();
}

Command ArgParser::getCommand() const {
    return _command;
}

const std::string &ArgParser::getPrompt() const {
    return _prompt;
}

const std::string &ArgParser::getOutputPath() const {
    return _outputPath;
}

const std::string &ArgParser::getImportFile() const {
    return _importFile;
}

const std::string &ArgParser::getRatingScore() const {
    return _ratingScore;
}

const std::string &ArgParser::getExerciseName() const {
    return _exerciseName;
}

bool ArgParser::shouldListGenerated() const {
    return _listGenerated;
}

bool ArgParser::shouldListShareable() const {
    return _listShareable;
}

bool ArgParser::metadataOnly() const {
    return _metadataOnly;
}
