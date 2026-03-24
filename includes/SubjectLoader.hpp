#ifndef SUBJECTLOADER_HPP
#define SUBJECTLOADER_HPP

#include <string>
#include <vector>

class SubjectLoader {
private:
    std::string _rank;
    std::string _level;
    std::string _subject;
    std::string _subjectContent;

public:
    SubjectLoader();
    SubjectLoader(const SubjectLoader &other);
    SubjectLoader &operator=(const SubjectLoader &other);
    ~SubjectLoader();

    bool load(const std::string &rank, const std::string &level, const std::string &subject);
    const std::string &getSubjectContent() const;

    std::vector<std::string> listRanks() const;
    std::vector<std::string> listLevels(const std::string &rank) const;
    std::vector<std::string> listSubjects(const std::string &rank, const std::string &level) const;
    std::vector<std::string> listGeneratedExercises(const std::string &rank) const;

    void showAvailableRanks() const;
    void showAvailableLevels(const std::string &rank) const;
    void showAvailableSubjects(const std::string &rank, const std::string &level) const;
    void showGeneratedExercises(const std::string &rank) const;
    void showAll() const;

private:
    bool directoryExists(const std::string &path) const;
    std::vector<std::string> listTxtFiles(const std::string &dirPath) const;
    std::string readFile(const std::string &path) const;
    bool isValidName(const std::string &name) const;
    std::vector<std::string> listDirectories(const std::string &path) const;
    bool loadGeneratedExercise(const std::string &rank, const std::string &subject);
};

#endif
