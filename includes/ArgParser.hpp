#ifndef ARGPARSER_HPP
#define ARGPARSER_HPP

#include <string>

enum Command {
    CMD_NONE,
    CMD_PRACTICE,
    CMD_GENERATE,
    CMD_SHARE,
    CMD_IMPORT,
    CMD_LIST,
    CMD_RATE,
    CMD_INFO
};

class ArgParser {
private:
    Command _command;
    std::string _rank;
    std::string _level;
    std::string _subject;
    std::string _sourceFile;
    std::string _prompt;
    std::string _outputPath;
    std::string _importFile;
    std::string _ratingScore;
    std::string _exerciseName;
    bool _showHelp;
    bool _listRanks;
    bool _listLevels;
    bool _listSubjects;
    bool _listGenerated;
    bool _listShareable;
    bool _showAll;
    bool _metadataOnly;

public:
    ArgParser();
    ArgParser(const ArgParser &other);
    ArgParser &operator=(const ArgParser &other);
    ~ArgParser();

    bool parse(int argc, char **argv);
    void showUsage() const;

    Command getCommand() const;
    bool hasRank() const;
    bool hasLevel() const;
    bool hasSubject() const;
    const std::string &getRank() const;
    const std::string &getLevel() const;
    const std::string &getSubject() const;
    const std::string &getSourceFile() const;
    const std::string &getPrompt() const;
    const std::string &getOutputPath() const;
    const std::string &getImportFile() const;
    const std::string &getRatingScore() const;
    const std::string &getExerciseName() const;
    bool shouldShowHelp() const;
    bool shouldListRanks() const;
    bool shouldListLevels() const;
    bool shouldListSubjects() const;
    bool shouldListGenerated() const;
    bool shouldListShareable() const;
    bool shouldShowAll() const;
    bool metadataOnly() const;

private:
    bool fileExists(const std::string &path) const;
};

#endif
