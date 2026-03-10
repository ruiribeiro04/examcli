#include "FileReader.hpp"

FileReader::FileReader() {
    std::cout << "FileReader default constructor called\n";
}

FileReader::FileReader(const FileReader &/*other*/) {
    std::cout << "FileReader copy constructor called\n";
}

FileReader &FileReader::operator=(const FileReader &/*other*/) {
    std::cout << "FileReader copy assignment operator called\n";
    return *this;
}

FileReader::~FileReader() {
    std::cout << "FileReader destructor called\n";
}

std::string FileReader::read(const std::string &path) const {
    std::ifstream file(path.c_str());
    
    std::string content;
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Error: could not open file: " << path << "\n";
        return "";
    }
    while (std::getline(file, line)) {
        content += line;
        content += "\n";
    }

    file.close();
    return content;
}
