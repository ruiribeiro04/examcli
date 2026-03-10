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
