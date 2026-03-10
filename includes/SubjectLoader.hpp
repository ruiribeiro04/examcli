#ifndef SUBJECTLOADER_HPP
#define SUBJECTLOADER_HPP

#include <string>
#include <vector>

class SubjectLoader {
private:
    std::string _exam;
    std::string _exercise;
    std::string _subjectContent;

public:
    SubjectLoader();
    SubjectLoader(const SubjectLoader &other);
    SubjectLoader &operator=(const SubjectLoader &other);
    ~SubjectLoader();

    bool load(const std::string &exam, const std::string &exercise);
    const std::string &getSubjectContent() const;

private:
    bool directoryExists(const std::string &path) const;
    std::vector<std::string> listTxtFiles(const std::string &dirPath) const;
    std::string readFile(const std::string &path) const;
    bool isValidName(const std::string &name) const;
};

#endif
