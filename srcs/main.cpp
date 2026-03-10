#include <iostream>
#include <cstdlib>
#include "ArgParser.hpp"
#include "SubjectLoader.hpp"
#include "FileReader.hpp"
#include "LLMClient.hpp"
#include "Result.hpp"

int main(int argc, char **argv) {
    ArgParser parser;

    if (!parser.parse(argc, argv)) {
        return 1;
    }

    if (parser.shouldShowHelp()) {
        return 0;
    }

    std::string exam = parser.hasExam() ? parser.getExam() : "exam02";
    std::string subject = parser.hasSubject() ? parser.getSubject() : "ex00";
    std::string sourceFile = parser.getSourceFile();

    SubjectLoader loader;
    if (!loader.load(exam, subject)) {
        return 1;
    }

    FileReader reader;
    std::string code = reader.read(sourceFile);

    if (code.empty()) {
        std::cerr << "Error: Could not read source file: " << sourceFile << "\n";
        return 1;
    }

    LLMClient client;
    if (!client.initialize()) {
        std::cerr << "Error: " << client.getLastError() << "\n";
        return 1;
    }

    bool isCorrect = false;
    std::string hint;

    std::cout << "Submitting code for correction...\n";

    if (!client.correct(loader.getSubjectContent(), code, isCorrect, hint)) {
        std::cerr << "Error: " << client.getLastError() << "\n";
        return 1;
    }

    Result result;
    result.setCorrectionResult(isCorrect, hint);
    result.show();

    return result.getExitCode();
}
