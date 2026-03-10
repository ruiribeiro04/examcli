#include "FileReader.hpp"

FileReader::FileReader() {
}

FileReader::FileReader(const FileReader &/*other*/) {
}

FileReader &FileReader::operator=(const FileReader &/*other*/) {
    return *this;
}

FileReader::~FileReader() {
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
