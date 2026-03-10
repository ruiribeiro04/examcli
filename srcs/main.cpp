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

    SubjectLoader loader;

    if (parser.shouldShowAll()) {
        loader.showAll();
        return 0;
    }

    if (parser.shouldListRanks()) {
        loader.showAvailableRanks();
        return 0;
    }

    if (parser.shouldListLevels()) {
        if (!parser.hasRank()) {
            std::cerr << "Error: --list-levels requires a rank (-r)\n";
            parser.showUsage();
            return 1;
        }
        loader.showAvailableLevels(parser.getRank());
        return 0;
    }

    if (parser.shouldListSubjects()) {
        if (!parser.hasRank() || !parser.hasLevel()) {
            std::cerr << "Error: --list-subjects requires a rank (-r) and level (-l)\n";
            parser.showUsage();
            return 1;
        }
        loader.showAvailableSubjects(parser.getRank(), parser.getLevel());
        return 0;
    }

    std::string rank = parser.hasRank() ? parser.getRank() : "rank02";
    std::string level = parser.hasLevel() ? parser.getLevel() : "level0";
    std::string subject = parser.hasSubject() ? parser.getSubject() : "fizzbuzz";
    std::string sourceFile = parser.getSourceFile();

    if (!loader.load(rank, level, subject)) {
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
