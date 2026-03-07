#ifndef FILEREADER_HPP
#define FILEREADER_HPP

#include <string>
#include <iostream>
#include <fstream>

class FileReader {
private:

public:
    FileReader();
    FileReader(const FileReader &other);
    FileReader &operator=(const FileReader &other);
    ~FileReader();

    std::string read(const std::string &path) const;
};

#endif
