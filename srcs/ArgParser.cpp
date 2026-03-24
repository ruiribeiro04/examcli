#include "ArgParser.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>

static std::string toLower(const std::string &s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                  static_cast<int(*)(int)>(std::tolower));
    return result;
}

ArgParser::ArgParser()
    : _command(CMD_NONE), _listSubcommand(), _rank(), _level(), _subject(),
      _sourceFile(), _prompt(), _outputPath(), _importFile(), _ratingScore(),
      _exerciseName(), _showHelp(false), _showAll(false), _metadataOnly(false),
      _verbose(false) {
}

ArgParser::ArgParser(const ArgParser &other)
    : _command(other._command), _listSubcommand(other._listSubcommand),
      _rank(other._rank), _level(other._level), _subject(other._subject),
      _sourceFile(other._sourceFile), _prompt(other._prompt),
      _outputPath(other._outputPath), _importFile(other._importFile),
      _ratingScore(other._ratingScore), _exerciseName(other._exerciseName),
      _showHelp(other._showHelp), _showAll(other._showAll),
      _metadataOnly(other._metadataOnly), _verbose(other._verbose) {
}

ArgParser &ArgParser::operator=(const ArgParser &other) {
    if (this != &other) {
        _command = other._command;
        _listSubcommand = other._listSubcommand;
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
        _showAll = other._showAll;
        _metadataOnly = other._metadataOnly;
        _verbose = other._verbose;
    }
    return *this;
}

ArgParser::~ArgParser() {
}

bool ArgParser::parse(int argc, char **argv) {
    if (argc < 2) {
        _showHelp = true;
        showUsage();
        return false;
    }

    if (!parseCommand(argc, argv)) {
        return false;
    }

    if (!parseCommandSpecificArgs(argc, argv)) {
        return false;
    }

    return validateArgs();
}

bool ArgParser::parseCommand(int argc, char **argv) {
    (void)argc;
    std::string cmd = toLower(argv[1]);

    if (cmd == "practice" || cmd == "submit") {
        _command = CMD_PRACTICE;
    } else if (cmd == "generate") {
        _command = CMD_GENERATE;
    } else if (cmd == "share") {
        _command = CMD_SHARE;
    } else if (cmd == "import") {
        _command = CMD_IMPORT;
    } else if (cmd == "list") {
        _command = CMD_LIST;
    } else if (cmd == "rate") {
        _command = CMD_RATE;
    } else if (cmd == "info") {
        _command = CMD_INFO;
    } else if (cmd == "--help" || cmd == "-h") {
        _showHelp = true;
        showUsage();
        return false;
    } else {
        std::cerr << "Error: Unknown command: " << argv[1] << "\n";
        std::string suggestion = suggestCommand(argv[1]);
        if (!suggestion.empty()) {
            std::cerr << "Hint: Did you mean '" << suggestion << "'?\n";
        }
        showUsage();
        return false;
    }

    return true;
}

bool ArgParser::parseCommandSpecificArgs(int argc, char **argv) {
    int i = 2;

    while (i < argc) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            _showHelp = true;
            showCommandSpecificHelp(_command);
            return false;
        } else if (arg == "--verbose" || arg == "-v") {
            _verbose = true;
            i++;
        } else if (_command == CMD_LIST && i == 2) {
            if (arg[0] != '-') {
                _listSubcommand = arg;
                if (!isValidListSubcommand(_listSubcommand)) {
                    std::cerr << "Error: Invalid list subcommand: " << arg << "\n";
                    std::cerr << "Hint: Valid subcommands are: " << getValidSubcommands() << "\n";
                    return false;
                }
                i++;
            } else {
                std::cerr << "Error: Missing list subcommand\n";
                std::cerr << "Hint: Use 'list ranks', 'list levels', 'list subjects', 'list generated', or 'list shareable'\n";
                return false;
            }
        } else if (arg == "--rank" || arg == "-r") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --rank requires an argument\n";
                std::cerr << "Hint: Use -r <rank_name> (e.g., -r rank02)\n";
                return false;
            }
            _rank = argv[++i];
            i++;
        } else if (arg == "--level" || arg == "-l") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --level requires an argument\n";
                std::cerr << "Hint: Use -l <level_name> (e.g., -l level0)\n";
                return false;
            }
            _level = argv[++i];
            i++;
        } else if (arg == "--subject" || arg == "-s") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --subject requires an argument\n";
                std::cerr << "Hint: Use -s <subject_name> (e.g., -s fizzbuzz)\n";
                return false;
            }
            _subject = argv[++i];
            i++;
        } else if (_command == CMD_GENERATE && (arg == "--type" || arg == "-t")) {
            if (i + 1 >= argc) {
                std::cerr << "Error: --type requires an argument\n";
                std::cerr << "Hint: Use --type <function|prog> or -t <function|prog>\n";
                return false;
            }
            _prompt += " type:" + std::string(argv[++i]);
            i++;
        } else if (_command == CMD_SHARE && (arg == "--output" || arg == "-o")) {
            if (i + 1 >= argc) {
                std::cerr << "Error: --output requires an argument\n";
                std::cerr << "Hint: Use -o <path> or --output <path>\n";
                return false;
            }
            _outputPath = argv[++i];
            i++;
        } else if (_command == CMD_SHARE && (arg == "--metadata-only" || arg == "-m")) {
            _metadataOnly = true;
            i++;
        } else if (arg == "--generated" || arg == "-g") {
            if (_command == CMD_LIST) {
                _listSubcommand = "generated";
                i++;
            } else {
                std::cerr << "Error: --generated is only valid with 'list' command\n";
                return false;
            }
        } else if (arg == "--shareable") {
            if (_command == CMD_LIST) {
                _listSubcommand = "shareable";
                i++;
            } else {
                std::cerr << "Error: --shareable is only valid with 'list' command\n";
                return false;
            }
        } else if (arg[0] == '-') {
            std::cerr << "Error: Unknown argument: " << arg << "\n";
            std::cerr << "Hint: Use --help to see available options\n";
            return false;
        } else {
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
            } else if (_command == CMD_PRACTICE) {
                if (_sourceFile.empty()) {
                    _sourceFile = arg;
                }
            }
            i++;
        }
    }

    return true;
}

bool ArgParser::validateArgs() {
    if (_command == CMD_PRACTICE) {
        if (_sourceFile.empty()) {
            std::cerr << "Error: practice command requires a source file\n";
            std::cerr << "Hint: Usage: ./examcli practice <source_file> [options]\n";
            showCommandSpecificHelp(CMD_PRACTICE);
            return false;
        }
        if (!fileExists(_sourceFile)) {
            std::cerr << "Error: File not found: " << _sourceFile << "\n";
            std::cerr << "Hint: Check the file path and try again\n";
            return false;
        }
    } else if (_command == CMD_GENERATE) {
        if (_prompt.empty()) {
            std::cerr << "Error: generate command requires a prompt\n";
            std::cerr << "Hint: Usage: ./examcli generate \"<prompt>\" [options]\n";
            showCommandSpecificHelp(CMD_GENERATE);
            return false;
        }
    } else if (_command == CMD_SHARE) {
        if (_exerciseName.empty()) {
            std::cerr << "Error: share command requires an exercise name\n";
            std::cerr << "Hint: Usage: ./examcli share <exercise_name> [options]\n";
            showCommandSpecificHelp(CMD_SHARE);
            return false;
        }
    } else if (_command == CMD_IMPORT) {
        if (_importFile.empty()) {
            std::cerr << "Error: import command requires a file path\n";
            std::cerr << "Hint: Usage: ./examcli import <file.tar.gz> [options]\n";
            showCommandSpecificHelp(CMD_IMPORT);
            return false;
        }
    } else if (_command == CMD_RATE) {
        if (_exerciseName.empty() || _ratingScore.empty()) {
            std::cerr << "Error: rate command requires exercise name and score\n";
            std::cerr << "Hint: Usage: ./examcli rate <exercise_name> <score>\n";
            showCommandSpecificHelp(CMD_RATE);
            return false;
        }
        int score = atoi(_ratingScore.c_str());
        if (score < 1 || score > 5) {
            std::cerr << "Error: Score must be between 1 and 5\n";
            return false;
        }
    } else if (_command == CMD_INFO) {
        if (_exerciseName.empty()) {
            std::cerr << "Error: info command requires an exercise name\n";
            std::cerr << "Hint: Usage: ./examcli info <exercise_name>\n";
            showCommandSpecificHelp(CMD_INFO);
            return false;
        }
    } else if (_command == CMD_LIST) {
        if (_listSubcommand == "levels" && _rank.empty()) {
            std::cerr << "Error: 'list levels' requires a rank (-r)\n";
            return false;
        }
        if (_listSubcommand == "subjects" && (_rank.empty() || _level.empty())) {
            std::cerr << "Error: 'list subjects' requires both rank (-r) and level (-l)\n";
            return false;
        }
    }

    return true;
}

void ArgParser::showQuickStart() const {
    std::cout << "Quick Start:\n"
              << "  1. Practice:     ./examcli practice solution.c -r rank02 -l level0 -s fizzbuzz\n"
              << "  2. Generate:     ./examcli generate \"linked list\" -r rank03\n"
              << "  3. List:         ./examcli list ranks\n"
              << "\n";
}

void ArgParser::showCommandSpecificHelp(Command cmd) const {
    switch (cmd) {
        case CMD_PRACTICE:
            std::cout << "\nUsage: ./examcli practice <source_file> [options]\n\n"
                      << "Practice mode - submit your solution for correction.\n\n"
                      << "Options:\n"
                      << "  -r, --rank <name>     Rank name (e.g., rank02)\n"
                      << "  -l, --level <name>    Level name (e.g., level0)\n"
                      << "  -s, --subject <name>  Subject name (e.g., fizzbuzz)\n"
                      << "  -v, --verbose         Verbose output\n"
                      << "  -h, --help            Show this help\n\n"
                      << "Examples:\n"
                      << "  ./examcli practice solution.c -r rank02 -l level0 -s fizzbuzz\n"
                      << "  ./examcli practice my_code.c -s linked_list -v\n";
            break;
        case CMD_GENERATE:
            std::cout << "\nUsage: ./examcli generate <prompt> [options]\n\n"
                      << "Generate a new exercise from prompt.\n\n"
                      << "Options:\n"
                      << "  -r, --rank <name>     Target rank (default: rank02)\n"
                      << "  -l, --level <name>    Target level (default: level0)\n"
                      << "  -t, --type <type>     Exercise type (function|prog)\n"
                      << "  -v, --verbose         Verbose output\n"
                      << "  -h, --help            Show this help\n\n"
                      << "Examples:\n"
                      << "  ./examcli generate \"linked list with insert\"\n"
                      << "  ./examcli generate \"sorting algorithm\" --type prog -r rank03\n";
            break;
        case CMD_SHARE:
            std::cout << "\nUsage: ./examcli share <exercise_name> [options]\n\n"
                      << "Export exercise as tar.gz archive.\n\n"
                      << "Options:\n"
                      << "  -o, --output <path>   Output file path\n"
                      << "  -m, --metadata-only  Export metadata only\n"
                      << "  -v, --verbose         Verbose output\n"
                      << "  -h, --help            Show this help\n\n"
                      << "Examples:\n"
                      << "  ./examcli share my_exercise\n"
                      << "  ./examcli share my_exercise -o /tmp/exercise.tar.gz\n"
                      << "  ./examcli share my_exercise -m\n";
            break;
        case CMD_IMPORT:
            std::cout << "\nUsage: ./examcli import <file.tar.gz> [options]\n\n"
                      << "Import exercise from archive.\n\n"
                      << "Options:\n"
                      << "  -r, --rank <name>    Override target rank\n"
                      << "  -v, --verbose         Verbose output\n"
                      << "  -h, --help            Show this help\n\n"
                      << "Examples:\n"
                      << "  ./examcli import exercise.tar.gz\n"
                      << "  ./examcli import exercise.tar.gz -r rank04\n";
            break;
        case CMD_LIST:
            std::cout << "\nUsage: ./examcli list <subcommand> [options]\n\n"
                      << "List available exercises, ranks, levels, or subjects.\n\n"
                      << "Subcommands:\n"
                      << "  ranks           List all available ranks\n"
                      << "  levels          List levels for a rank (requires -r)\n"
                      << "  subjects        List subjects for rank/level (requires -r -l)\n"
                      << "  generated       List generated exercises\n"
                      << "  shareable       List shareable exercises\n\n"
                      << "Options:\n"
                      << "  -r, --rank <name>   Rank name\n"
                      << "  -l, --level <name>   Level name\n"
                      << "  -g, --generated     List generated (short form)\n"
                      << "  -v, --verbose        Verbose output\n"
                      << "  -h, --help           Show this help\n\n"
                      << "Examples:\n"
                      << "  ./examcli list ranks\n"
                      << "  ./examcli list levels -r rank02\n"
                      << "  ./examcli list subjects -r rank02 -l level0\n"
                      << "  ./examcli list generated\n";
            break;
        case CMD_RATE:
            std::cout << "\nUsage: ./examcli rate <exercise_name> <score>\n\n"
                      << "Rate an exercise (1-5).\n\n"
                      << "Options:\n"
                      << "  -v, --verbose       Verbose output\n"
                      << "  -h, --help          Show this help\n\n"
                      << "Examples:\n"
                      << "  ./examcli rate my_exercise 5\n"
                      << "  ./examcli rate fizzbuzz 4\n";
            break;
        case CMD_INFO:
            std::cout << "\nUsage: ./examcli info <exercise_name>\n\n"
                      << "Show exercise details.\n\n"
                      << "Options:\n"
                      << "  -v, --verbose       Verbose output\n"
                      << "  -h, --help          Show this help\n\n"
                      << "Examples:\n"
                      << "  ./examcli info my_exercise\n"
                      << "  ./examcli info fizzbuzz\n";
            break;
        default:
            showUsage();
            break;
    }
}

void ArgParser::showUsage() const {
    showQuickStart();

    std::cout << "\nCommands:\n"
              << "  practice, submit  Practice mode - submit solution for correction\n"
              << "  generate          Generate a new exercise from prompt\n"
              << "  share             Export exercise as tar.gz archive\n"
              << "  import            Import exercise from archive\n"
              << "  list               List available exercises, ranks, levels, subjects\n"
              << "  rate               Rate an exercise (1-5)\n"
              << "  info               Show exercise details\n\n"
              << "Global Options:\n"
              << "  -v, --verbose      Verbose output\n"
              << "  -h, --help         Show help\n\n"
              << "Use './examcli <command> --help' for command-specific help.\n";
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

const std::string &ArgParser::getListSubcommand() const {
    return _listSubcommand;
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

bool ArgParser::metadataOnly() const {
    return _metadataOnly;
}

bool ArgParser::verbose() const {
    return _verbose;
}

bool ArgParser::isValidListSubcommand(const std::string &subcmd) const {
    return subcmd == "ranks" || subcmd == "levels" ||
           subcmd == "subjects" || subcmd == "generated" ||
           subcmd == "shareable";
}

bool ArgParser::isValidGenerateFlag(const std::string &flag) const {
    return flag == "--rank" || flag == "-r" ||
           flag == "--level" || flag == "-l" ||
           flag == "--type" || flag == "-t" ||
           flag == "--help" || flag == "-h";
}

std::string ArgParser::suggestCommand(const std::string &input) const {
    std::string cmd = toLower(input);
    std::string commands[] = {"practice", "generate", "share", "import",
                              "list", "rate", "info"};

    for (size_t i = 0; i < sizeof(commands)/sizeof(commands[0]); ++i) {
        size_t matches = 0;
        size_t minLen = std::min(cmd.size(), commands[i].size());
        for (size_t j = 0; j < minLen; ++j) {
            if (cmd[j] == commands[i][j]) {
                matches++;
            }
        }
        if (matches >= minLen - 1 && minLen > 2) {
            return commands[i];
        }
    }
    return "";
}

std::string ArgParser::getValidSubcommands() const {
    if (_command == CMD_LIST) {
        return "ranks, levels, subjects, generated, shareable";
    }
    return "";
}
