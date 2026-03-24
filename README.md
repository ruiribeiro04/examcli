# examcli

A CLI tool to practice 42 exams with LLM-powered feedback. Get your code evaluated without the temptation of having AI write it for you.

## What does it do?

You write the code yourself, submit it, and get instant feedback on whether it meets the subject requirements. The LLM only evaluates - it doesn't generate code, so you still do all the work.

## Quick Start

```bash
# Clone it
git clone https://github.com/ruiribeiro04/examcli.git
cd examcli

# Build it
make

# Set up your LLM (required)
export LLM_API_KEY="your-api-key"
export LLM_MODEL="model_name"
export LLM_API_URL="https://example.com/v1/chat/completions"

# See all available exercises
./examcli

# Run it
./examcli my_solution.cpp
```

## Usage

Basic usage:
```bash
./examcli code.cpp
```

Specify rank, level and subject:
```bash
./examcli -r rank02 -l level0 -s fizzbuzz code.c
```

Use short flags:
```bash
./examcli -r rank03 -l level1 -s filter solution.c
```

See all options:
```bash
./examcli -h
```

### Listing Available Ranks, Levels, and Subjects

When you get an error about a rank/level/subject not being found, the tool now automatically shows you what's available. You can also list them explicitly:

List all available ranks:
```bash
./examcli --list-ranks
# or
./examcli --lr
```

List levels for a specific rank:
```bash
./examcli -r rank04 --list-levels
# or
./examcli -r rank04 --ll
```

List subjects for a specific rank and level:
```bash
./examcli -r rank04 -l level1 --list-subjects
# or
./examcli -r rank04 -l level1 --ls
```

## Generating Custom Exercises

examcli can generate custom 42-style exercises from natural language descriptions using AI. This feature allows you to create unlimited practice exercises tailored to your learning needs.

### Generate Command

Basic usage:
```bash
./examcli generate "linked list with insert and delete functions"
```

Generate with specific rank and level:
```bash
./examcli generate "binary search tree" --rank rank03 --level level1
```

Generate a program exercise (instead of function):
```bash
./examcli generate "simple text editor" --type program
```

### What Gets Generated

When you generate an exercise, examcli creates:

1. **Subject Description** (`attachment/subject.en.txt`) - Clear exercise requirements with examples
2. **Reference Solution** (`solution.c` or `.cpp`) - Clean, compilable implementation
3. **Test Harness** (`tester.sh`) - Comprehensive test cases using 42-grademe format
4. **Metadata** (`metadata.json`) - Generation details, quality scores, validation info
5. **Validation Artifacts** (`.validation/`) - Compilation logs, test results, quality reports

### Quality Scoring System

Every generated exercise is automatically validated and scored (0-100 scale):

- **Compilation (25%)**: 100 points if clean, 70 if warnings, 0 if errors
- **Test Coverage (30%)**: Normal cases (30pts), edge cases (40pts), error cases (30pts)
- **Subject Clarity (25%)**: Length, examples, requirements, completeness
- **Code Quality (20%)**: 42 norms adherence, memory management, error handling, organization

**Approval Threshold**: Exercises need ≥70/100 to be saved. Lower-scoring exercises are rejected with detailed feedback.

### Validation Pipeline

Generated exercises go through a 6-stage validation process:

1. **LLM Generation** - Creates structured JSON with all exercise components
2. **Syntax Validation** - Verifies JSON structure and required fields
3. **Compilation** - Compiles solution with strict gcc flags (`-std=c99 -Wall -Wextra -Werror`)
4. **Execution Testing** - Runs solution against all test cases
5. **Test Generation** - Generates `tester.sh` from test cases
6. **Final Validation** - Full cycle: compile, test, valgrind (memory leaks), infinite loop detection

Each stage allows up to 3 retry attempts with refinement prompts.

### Example Prompts

Good prompts for different exercise types:

**Function exercises:**
```bash
./examcli generate "string to uppercase conversion function"
./examcli generate "array sorting with bubble sort"
./examcli generate "calculate fibonacci number recursively"
```

**Program exercises:**
```bash
./examcli generate "simple calculator that handles + - * /" --type program
./examcli generate "phonebook that can add and search contacts" --type program
```

**Data structures:**
```bash
./examcli generate "singly linked list with insert delete and search"
./examcli generate "stack implementation using arrays with push pop"
./examcli generate "queue with enqueue dequeue operations"
```

**Algorithms:**
```bash
./examcli generate "binary search implementation on sorted array"
./examcli generate "find duplicate in array of integers"
./examcli generate "check if string is palindrome"
```

### Constraints and Rank-Specific Norms

The system automatically applies 42 School coding norms based on rank:

- **rank02**: No loops (for/do-while), recursion only, no typedef, no switch
- **rank03+**: Proper memory management, error handling, Orthodox Canonical Form for classes

You can customize constraints by creating a `constraints.yaml` file:

```yaml
rank02:
  forbidden:
    - for loops
    - do-while loops
    - typedef
    - switch statements
  required:
    - functions only
    - recursion
  max_lines: 50

rank03:
  forbidden:
    - STL containers
    - exceptions
  required:
    - manual memory management
```

### Exporting and Sharing Exercises

Export a generated exercise to share with others:
```bash
./examcli share user_linked_list
# Creates: user_linked_list.tar.gz
```

Export with custom output path:
```bash
./examcli share user_linked_list -o ~/my_exercises/
```

Export metadata only (for quick preview):
```bash
./examcli share user_linked_list --metadata-only
```

### Importing Exercises

Import an exercise shared by someone else:
```bash
./examcli import user_linked_list.tar.gz
```

Import with rank override:
```bash
./examcli import user_linked_list.tar.gz --rank rank04
```

### Listing Generated Exercises

List all generated exercises:
```bash
./examcli list --generated
```

List shareable exercises (that can be exported):
```bash
./examcli list --shareable
```

### Rating and Info

Rate a generated exercise (1-5 scale):
```bash
./examcli rate user_linked_list 5
```

View exercise information:
```bash
./examcli info user_linked_list
```

This displays:
- Quality score breakdown
- Generation details (model, prompt, timestamp)
- Ratings and statistics
- Validation rounds
- File locations

### Metadata Structure

Each generated exercise has a `metadata.json` file:

```json
{
  "exercise_name": "user_linked_list",
  "rank": "rank02",
  "level": "level0",
  "exercise_type": "function",
  "generated_at": "2026-03-24 12:00:00",
  "llm_model": "gpt-4",
  "prompt": "linked list with insert and delete",
  "quality_scores": {
    "overall": 85,
    "compilation": 100,
    "test_coverage": 80,
    "subject_clarity": 90,
    "code_quality": 75
  },
  "validation_rounds": 2,
  "approved": true,
  "ratings": [
    {"user": "alice", "score": 5},
    {"user": "bob", "score": 4}
  ],
  "average_rating": 4.5,
  "rating_count": 2,
  "shareable": true
}
```

### Troubleshooting Generation Issues

**Generation fails after multiple retries:**
- Try rephrasing your prompt to be more specific
- Simplify the exercise requirements
- Check your LLM API configuration
- Try again later if the API is experiencing issues

**Low quality score (< 70):**
- Review the quality breakdown to see which metrics scored poorly
- The system will show specific suggestions for improvement
- Try regenerating with a more detailed prompt

**Missing .system/ scripts warning:**
- The tool will warn if `.system/auto_correc_main.sh` or `.system/auto_correc_program.sh` are missing
- These templates are required for `tester.sh` to work correctly
- Copy them from a 42-grademe repository or create your own

**Permission errors when saving:**
- Ensure you have write permissions for the `subjects/generated/` directory
- Check that parent directories exist
- Verify disk space is available

## Configuration

You need to set these environment variables:

| Variable | Description |
|----------|-------------|
| `LLM_API_KEY` | Your API key |
| `LLM_MODEL` | Model name |
| `LLM_API_URL` | API endpoint URL |

**Tip:** Add them to your `.bashrc` or `.zshrc` so you don't have to export them every time.

## How it works

1. Reads your source file
2. Loads the subject description from `subjects/<rank>/<level>/<subject>/`
3. Sends both to the LLM with a system prompt
4. Parses the response and shows you pass/fail + hints

The LLM knows the subject requirements and checks if your code actually does what it should. The key point: it only evaluates, it never writes code for you.

## Project Structure

```
examcli/
├── srcs/           # Source files
├── includes/       # Headers
├── subjects/       # Subject descriptions (txt files)
├── objs/           # Object files (created by make)
└── examcli         # The binary
```

## Requirements

- C++98 compiler (g++ or clang++)
- libcurl
- A working LLM API endpoint

## Notes

- **You need to add your subjects manually** - just drop txt files in the subjects folder
- **LLM isn't perfect** - it might miss edge cases or be too strict. Use your judgment.

## Troubleshooting

**"Error: Could not initialize LLM client"**
→ Check your environment variables are set correctly.

**"Subject not found"**
→ The tool will automatically show available subjects for that rank/level. You can also use `./examcli -r <rank> -l <level> --list-subjects` to see all options.

## TODO / Ideas

- [ ] Support for multiple subject files per exercise
- [ ] Better error messages from LLM responses
- [ ] Maybe add a progress tracker/exam mode?

## License

This project is licensed under the [GNU General Public License v3.0](LICENSE).

---

Built to practice exams with objective feedback while keeping the coding 100% manual. No shortcuts.
