#include "SubjectLoader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdlib>
#include <ctime>

SubjectLoader::SubjectLoader() : _rank(), _level(), _subject(), _subjectContent() {
    std::srand(std::time(NULL));
}

SubjectLoader::SubjectLoader(const SubjectLoader &other)
    : _rank(other._rank), _level(other._level),
      _subject(other._subject), _subjectContent(other._subjectContent) {
}

SubjectLoader &SubjectLoader::operator=(const SubjectLoader &other) {
    if (this != &other) {
        _rank = other._rank;
        _level = other._level;
        _subject = other._subject;
        _subjectContent = other._subjectContent;
    }
    return *this;
}

SubjectLoader::~SubjectLoader() {
}

bool SubjectLoader::load(const std::string &rank, const std::string &level, const std::string &subject) {
    if (!isValidName(rank)) {
        std::cerr << "Error: Invalid rank name: " << rank << "\n";
        return false;
    }

    if (!isValidName(level)) {
        std::cerr << "Error: Invalid level name: " << level << "\n";
        return false;
    }

    if (!isValidName(subject)) {
        std::cerr << "Error: Invalid subject name: " << subject << "\n";
        return false;
    }

    _rank = rank;
    _level = level;
    _subject = subject;

    std::string basePath = "subjects/";
    std::string rankPath = basePath + rank;

    if (!directoryExists(rankPath)) {
        std::cerr << "Error: Rank not found: " << rank << "\n";
        showAvailableRanks();
        return false;
    }

    std::string levelPath = rankPath + "/" + level;

    if (!directoryExists(levelPath)) {
        std::cerr << "Error: Level not found: " << level << "\n";
        showAvailableLevels(rank);
        return false;
    }

    std::string subjectPath = levelPath + "/" + subject;

    if (!directoryExists(subjectPath)) {
        std::cerr << "Error: Subject not found: " << subject << "\n";
        showAvailableSubjects(rank, level);
        return false;
    }

    std::vector<std::string> txtFiles = listTxtFiles(subjectPath);

    if (txtFiles.empty()) {
        std::cerr << "Error: No subject files (.txt) found for " << rank << "/" << level << "/" << subject << "\n";
        return false;
    }

    int randomIndex = std::rand() % txtFiles.size();
    std::string selectedFile = subjectPath + "/" + txtFiles[randomIndex];

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

std::vector<std::string> SubjectLoader::listDirectories(const std::string &path) const {
    std::vector<std::string> dirs;
    DIR *dir = opendir(path.c_str());

    if (dir == NULL) {
        return dirs;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name != "." && name != "..") {
            std::string fullPath = path + "/" + name;
            if (directoryExists(fullPath)) {
                dirs.push_back(name);
            }
        }
    }

    closedir(dir);
    return dirs;
}

std::vector<std::string> SubjectLoader::listRanks() const {
    return listDirectories("subjects");
}

std::vector<std::string> SubjectLoader::listLevels(const std::string &rank) const {
    return listDirectories("subjects/" + rank);
}

std::vector<std::string> SubjectLoader::listSubjects(const std::string &rank, const std::string &level) const {
    return listDirectories("subjects/" + rank + "/" + level);
}

void SubjectLoader::showAvailableRanks() const {
    std::vector<std::string> ranks = listRanks();
    std::cout << "\nAvailable ranks:\n";
    for (size_t i = 0; i < ranks.size(); ++i) {
        std::cout << "  " << ranks[i];
    }
    std::cout << "\n";
}

void SubjectLoader::showAvailableLevels(const std::string &rank) const {
    std::vector<std::string> levels = listLevels(rank);
    std::cout << "\nAvailable levels for " << rank << ":\n";
    for (size_t i = 0; i < levels.size(); ++i) {
        std::cout << "  " << levels[i];
    }
    std::cout << "\n";
}

void SubjectLoader::showAvailableSubjects(const std::string &rank, const std::string &level) const {
    std::vector<std::string> subjects = listSubjects(rank, level);
    std::cout << "\nAvailable subjects for " << rank << "/" << level << ":\n";
    for (size_t i = 0; i < subjects.size(); ++i) {
        std::cout << "  " << subjects[i];
    }
    std::cout << "\n";
}

void SubjectLoader::showAll() const {
    std::vector<std::string> ranks = listRanks();
    
    std::cout << "Available exercises:\n\n";
    
    for (size_t r = 0; r < ranks.size(); ++r) {
        std::cout << ranks[r] << "/\n";
        
        std::vector<std::string> levels = listLevels(ranks[r]);
        for (size_t l = 0; l < levels.size(); ++l) {
            std::cout << "  " << levels[l] << "/\n";
            
            std::vector<std::string> subjects = listSubjects(ranks[r], levels[l]);
            for (size_t s = 0; s < subjects.size(); ++s) {
                std::cout << "    " << subjects[s] << "\n";
            }
        }
    }
    
    std::cout << "\nUsage: ./examcli [options] <source_file>\n";
    std::cout << "Use -h for help\n";
}
