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

# Run it
./examcli my_solution.cpp
```

## Usage

Basic usage:
```bash
./examcli code.cpp
```

Specify exam and subject:
```bash
./examcli -e exam03 -s ex03 code.cpp
```

See all options:
```bash
./examcli -h
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
2. Loads the subject description from `subjects/<exam>/<subject>/`
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
→ Make sure the subject file exists in `subjects/<exam>/<subject>/subject1.txt`

## TODO / Ideas

- [ ] Support for multiple subject files per exercise
- [ ] Better error messages from LLM responses
- [ ] Maybe add a progress tracker/exam mode?

## License

This project is licensed under the [GNU General Public License v3.0](LICENSE).

---

Built to practice exams with objective feedback while keeping the coding 100% manual. No shortcuts.
