# examcli v2.0.0 Announcement

## 🚨 Breaking Changes

**examcli v2.0.0 is now available!** This release introduces a major CLI interface overhaul.

### What Changed

The CLI now uses a hierarchical command structure with explicit subcommands:

```bash
# OLD (v1.x)
./examcli --list-ranks
./examcli source.c  # implicit practice

# NEW (v2.0)
./examcli list ranks
./examcli practice source.c
```

### Migration

| Old | New |
|-----|-----|
| `./examcli --list-ranks` | `./examcli list ranks` |
| `./examcli source.c` | `./examcli practice source.c` |
| `./examcli` (show subjects) | `./examcli` (shows help) |

### New Features

- **Command-specific help**: `./examcli practice --help`
- **Quick Start guide**: Built into help output
- **Short flags**: `-t` for `--type`, `-v` for `--verbose`, etc.
- **Better error messages**: Shows suggestions when you make mistakes

### Upgrade

```bash
git pull
make clean
make
```

### Full Documentation

See [CHANGELOG.md](CHANGELOG.md) and [RELEASE_NOTES.md](RELEASE_NOTES.md) for details.
