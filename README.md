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

# See help with examples
./examcli --help

# Practice with an exercise
./examcli practice solution.c -r rank02 -l level0 -s fizzbuzz
```

## Usage

### Practice Mode

Practice mode allows you to submit your solution for correction:

```bash
# Basic practice
./examcli practice my_solution.cpp

# With specific rank, level, and subject
./examcli practice solution.c -r rank02 -l level0 -s fizzbuzz

# Use short flags
./examcli practice solution.c -r rank03 -l level1 -s filter
```

### List Command

The `list` command with subcommands replaces the old flag-based approach:

```bash
# List all available ranks
./examcli list ranks

# List levels for a specific rank
./examcli list levels -r rank04

# List subjects for a specific rank and level
./examcli list subjects -r rank04 -l level1

# List generated exercises
./examcli list generated

# List shareable exercises
./examcli list shareable

# Short form for generated
./examcli list -g
```

### Generate Command

Generate custom 42-style exercises using AI:

```bash
# Basic generation
./examcli generate "linked list with insert and delete functions"

# With specific rank and level
./examcli generate "binary search tree" -r rank03 -l level1

# Specify exercise type (function or program)
./examcli generate "simple text editor" -t prog
```

### Share Command

Export exercises as archives:

```bash
# Basic share
./examcli share my_exercise

# With custom output path
./examcli share my_exercise -o /tmp/exercise.tar.gz

# Metadata only
./examcli share my_exercise -m
# or
./examcli share my_exercise --metadata-only
```

### Import Command

Import exercises from archives:

```bash
./examcli import exercise.tar.gz
./examcli import exercise.tar.gz -r rank04
```

### Rate Command

Rate exercises you've completed:

```bash
./examcli rate my_exercise 5
./examcli rate fizzbuzz 4
```

### Info Command

Get details about an exercise:

```bash
./examcli info my_exercise
```

### Global Options

```bash
-v, --verbose      Enable verbose output
-h, --help         Show help
```

## Help System

Use `--help` with any command for specific guidance:

```bash
./examcli --help           # General help with Quick Start
./examcli practice --help   # Practice-specific help
./examcli list --help      # List subcommands
./examcli generate --help  # Generation options
```
