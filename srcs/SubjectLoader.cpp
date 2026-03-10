#include "SubjectLoader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdlib>
#include <ctime>

SubjectLoader::SubjectLoader() : _exam(), _exercise(), _subjectContent() {
    std::cout << "SubjectLoader default constructor called\n";
    std::srand(std::time(NULL));
}

SubjectLoader::SubjectLoader(const SubjectLoader &other)
    : _exam(other._exam), _exercise(other._exercise),
      _subjectContent(other._subjectContent) {
    std::cout << "SubjectLoader copy constructor called\n";
}

SubjectLoader &SubjectLoader::operator=(const SubjectLoader &other) {
    std::cout << "SubjectLoader copy assignment operator called\n";
    if (this != &other) {
        _exam = other._exam;
        _exercise = other._exercise;
        _subjectContent = other._subjectContent;
    }
    return *this;
}

SubjectLoader::~SubjectLoader() {
    std::cout << "SubjectLoader destructor called\n";
}

bool SubjectLoader::load(const std::string &exam, const std::string &exercise) {
    if (!isValidName(exam)) {
        std::cerr << "Error: Invalid exam name: " << exam << "\n";
        return false;
    }

    if (!isValidName(exercise)) {
        std::cerr << "Error: Invalid exercise name: " << exercise << "\n";
        return false;
    }

    _exam = exam;
    _exercise = exercise;

    std::string basePath = "subjects/";
    std::string examPath = basePath + exam;

    if (!directoryExists(examPath)) {
        std::cerr << "Error: Exam not found: " << exam << "\n";
        return false;
    }

    std::string exercisePath = examPath + "/" + exercise;

    if (!directoryExists(exercisePath)) {
        std::cerr << "Error: Exercise not found: " << exercise << "\n";
        return false;
    }

    std::vector<std::string> txtFiles = listTxtFiles(exercisePath);

    if (txtFiles.empty()) {
        std::cerr << "Error: No subjects found for " << exam << "/" << exercise << "\n";
        return false;
    }

    int randomIndex = std::rand() % txtFiles.size();
    std::string selectedFile = exercisePath + "/" + txtFiles[randomIndex];

    _subjectContent = readFile(selectedFile);

    if (_subjectContent.empty() && !txtFiles[randomIndex].empty()) {
        std::cerr << "Error: Cannot read subject file\n";
        return false;
    }

    return true;
}

const std::string &SubjectLoader::getSubjectContent() const {
    return _subjectContent;
}

bool SubjectLoader::directoryExists(const std::string &path) const {
    struct stat info;
    return stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
}

std::vector<std::string> SubjectLoader::listTxtFiles(const std::string &dirPath) const {
    std::vector<std::string> files;
    DIR *dir = opendir(dirPath.c_str());

    if (dir == NULL) {
        return files;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name.length() > 4 && name.substr(name.length() - 4) == ".txt") {
            files.push_back(name);
        }
    }

    closedir(dir);
    return files;
}

std::string SubjectLoader::readFile(const std::string &path) const {
    std::ifstream file(path.c_str());

    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

bool SubjectLoader::isValidName(const std::string &name) const {
    for (size_t i = 0; i < name.length(); ++i) {
        char c = name[i];
        if (!std::isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }
    return !name.empty();
}
