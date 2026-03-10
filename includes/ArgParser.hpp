#ifndef ARGPARSER_HPP
#define ARGPARSER_HPP

#include <string>

class ArgParser {
private:
    std::string _rank;
    std::string _level;
    std::string _subject;
    std::string _sourceFile;
    bool _showHelp;
    bool _listRanks;
    bool _listLevels;
    bool _listSubjects;
    bool _showAll;

public:
    ArgParser();
    ArgParser(const ArgParser &other);
    ArgParser &operator=(const ArgParser &other);
    ~ArgParser();

    bool parse(int argc, char **argv);
    void showUsage() const;

    bool hasRank() const;
    bool hasLevel() const;
    bool hasSubject() const;
    const std::string &getRank() const;
    const std::string &getLevel() const;
    const std::string &getSubject() const;
    const std::string &getSourceFile() const;
    bool shouldShowHelp() const;
    bool shouldListRanks() const;
    bool shouldListLevels() const;
    bool shouldListSubjects() const;
    bool shouldShowAll() const;

private:
    bool fileExists(const std::string &path) const;
};

#endif
