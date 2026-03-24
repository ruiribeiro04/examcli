# examcli v2.0.0 Release Notes

## Breaking Changes

**This release contains breaking changes to the CLI interface.**

### What's New

- **Hierarchical command structure**: Subcommands replace flat flags
- **Explicit practice mode**: Use `./examcli practice` instead of implicit mode
- **Standardized flags**: All flags now have both short and long forms
- **Better help**: Quick Start guide and command-specific help

### Migration

| Old | New |
|-----|-----|
| `./examcli --list-ranks` | `./examcli list ranks` |
| `./examcli source.c` | `./examcli practice source.c` |
| `./examcli` | `./examcli` (now shows help) |

### Quick Start

```bash
# Practice
./examcli practice <source> -r <rank> -l <level> -s <subject>

# List
./examcli list ranks
./examcli list levels -r <rank>

# Generate
./examcli generate "prompt" [-t <type>]

# Help
./examcli --help
./examcli <command> --help
```

### Full Documentation

See [CHANGELOG.md](CHANGELOG.md) for complete migration guide.
