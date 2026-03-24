#ifndef ARGPARSER_HPP
#define ARGPARSER_HPP

#include <string>

/**
 * @enum Command
 * @brief Supported command types for the CLI application.
 */
enum Command {
    CMD_NONE = 0,

    CMD_PRACTICE,

    CMD_GENERATE,
    CMD_SHARE,
    CMD_IMPORT,

    CMD_LIST,
    CMD_RATE,
    CMD_INFO
};

/**
 * @class ArgParser
 * @brief Command-line argument parser with command-first parsing approach.
 *
 * Supports hierarchical command structure with subcommands (e.g., 'list ranks').
 * All error messages include helpful suggestions for common mistakes.
 */
class ArgParser {
private:
    Command _command;
    std::string _listSubcommand;
    std::string _rank;
    std::string _level;
    std::string _subject;
    std::string _sourceFile;
    std::string _prompt;
    std::string _outputPath;
    std::string _importFile;
    std::string _ratingScore;
    std::string _exerciseName;
    bool _showHelp;
    bool _showAll;
    bool _metadataOnly;
    bool _verbose;

public:
    ArgParser();
    ArgParser(const ArgParser &other);
    ArgParser &operator=(const ArgParser &other);
    ~ArgParser();

    /**
     * @brief Parse command-line arguments using command-first approach.
     * @param argc Argument count from main().
     * @param argv Argument vector from main().
     * @return true if parsing succeeded and no errors, false otherwise.
     *
     * Parses argv[1] as command, then remaining args as command-specific flags.
     * Prints error messages with suggestions on failure.
     */
    bool parse(int argc, char **argv);

    /**
     * @brief Display full usage information organized by workflow sections.
     *
     * Shows commands grouped by category (Practice, Discovery, Management),
     * all flag options with both short and long forms, and usage examples.
     */
    void showUsage() const;

    /**
     * @brief Display quick start guide with 3 common workflows.
     *
     * Provides concise examples for the most common use cases to help
     * new users get started quickly.
     */
    void showQuickStart() const;

    /**
     * @brief Display help text specific to a command.
     * @param cmd The command to show help for.
     *
     * Shows available flags and examples for the specified command.
     */
    void showCommandSpecificHelp(Command cmd) const;

    Command getCommand() const;
    const std::string &getListSubcommand() const;
    bool hasRank() const;
    bool hasLevel() const;
    bool hasSubject() const;
    const std::string &getRank() const;
    const std::string &getLevel() const;
    const std::string &getSubject() const;
    const std::string &getSourceFile() const;
    const std::string &getPrompt() const;
    const std::string &getOutputPath() const;
    const std::string &getImportFile() const;
    const std::string &getRatingScore() const;
    const std::string &getExerciseName() const;
    bool shouldShowHelp() const;
    bool shouldShowAll() const;
    bool metadataOnly() const;

    /**
     * @brief Check if verbose output mode is enabled.
     * @return true if -v/--verbose flag was provided.
     */
    bool verbose() const;

private:
    bool fileExists(const std::string &path) const;

    /**
     * @brief Validate a list subcommand string.
     * @param subcmd The subcommand to validate.
     * @return true if valid (ranks, levels, subjects, generated, shareable).
     *
     * Used by 'list' command to validate its subcommands.
     */
    bool isValidListSubcommand(const std::string &subcmd) const;

    /**
     * @brief Check if a flag is valid for the generate command.
     * @param flag The flag to check.
     * @return true if flag is valid for generate command.
     */
    bool isValidGenerateFlag(const std::string &flag) const;

    /**
     * @brief Suggest a similar valid command for typos.
     * @param input The invalid command input.
     * @return A suggestion for the closest matching command.
     *
     * Uses simple string distance to find the most similar valid command.
     */
    std::string suggestCommand(const std::string &input) const;

    /**
     * @brief Get string listing all valid subcommands for error messages.
     * @return Space-separated list of valid subcommands.
     */
    std::string getValidSubcommands() const;

    /**
     * @brief Parse and validate the command from argv[1].
     * @param argc Argument count.
     * @param argv Argument vector.
     * @return true if command is valid, false otherwise.
     *
     * Sets _command and _listSubcommand based on argv[1].
     * Prints error with suggestion if command is invalid.
     */
    bool parseCommand(int argc, char **argv);

    /**
     * @brief Parse remaining arguments as command-specific flags.
     * @param argc Argument count.
     * @param argv Argument vector.
     * @return true if all flags are valid for the command, false otherwise.
     *
     * Handles flag parsing based on the command set by parseCommand().
     */
    bool parseCommandSpecificArgs(int argc, char **argv);

    /**
     * @brief Validate parsed arguments for command-specific requirements.
     * @return true if all required arguments are present and valid.
     *
     * Checks that required flags are provided for each command.
     */
    bool validateArgs();
};

#endif
