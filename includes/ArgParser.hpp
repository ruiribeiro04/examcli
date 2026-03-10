#ifndef ARGPARSER_HPP
#define ARGPARSER_HPP

#include <string>

class ArgParser {
private:
    std::string _exam;
    std::string _subject;
    std::string _sourceFile;
    bool _showHelp;

public:
    ArgParser();
    ArgParser(const ArgParser &other);
    ArgParser &operator=(const ArgParser &other);
    ~ArgParser();

    bool parse(int argc, char **argv);
    void showUsage() const;

    bool hasExam() const;
    bool hasSubject() const;
    const std::string &getExam() const;
    const std::string &getSubject() const;
    const std::string &getSourceFile() const;
    bool shouldShowHelp() const;

private:
    bool fileExists(const std::string &path) const;
};

#endif
