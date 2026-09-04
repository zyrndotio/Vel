# Changelog

All notable changes to the Vel programming language will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added
- Added a portable `vel check` command for syntax validation without NASM or a host linker.
- Added CTest smoke coverage and GitHub Actions builds for Linux, macOS, and Windows.
- Added a clearer runtime capability report to `vel version`.

### Changed
- Native compilation now writes generated artifacts beside the input `.vel` file and safely quotes paths.
- Native compilation now reports its Linux x86-64 limitation explicitly on other hosts.
- Implemented `continue` for `while` and `loop`, including stack cleanup for skipped scopes.

### Planned
- String concatenation with `+` operator
- Array types and indexing
- Better error messages with line/column diagnostics
- Type inference improvements
- Module system basics
- Windows target (PE/COFF) support
- macOS target (Mach-O) support

---

## [0.1.0] - 2026-09-03

### Added
- **Core Language Features**
  - Variable declarations with `let` (immutable) and `mut` (mutable)
  - Type annotations for `int`, `float`, `str`, `bool`
  - Arithmetic operators: `+`, `-`, `*`, `/`, `%`
  - Comparison operators: `==`, `!=`, `<`, `>`, `<=`, `>=`
  - Logical operators: `&&`, `||`, `!`
  - Assignment with `=`

- **Control Flow**
  - `if` / `elif` / `else` conditional statements
  - `while` loops with conditions
  - `loop` infinite loops with `break` statement
  - `continue` keyword for skipping iterations

- **Functions**
  - Function definitions with `fn` keyword
  - Parameter lists with type annotations
  - Return types with `->`
  - `return` statement
  - Function calls with arguments

- **Built-in Functions**
  - `print` for output (integers, strings, booleans)
  - Automatic newline after print statements

- **Compiler Infrastructure**
  - **Tokenizer**: Lexical analysis with token stream output
  - **Parser**: Recursive descent parser with AST construction
  - **Code Generator**: x86-64 Linux assembly generation
  - **Arena Allocator**: Memory management for AST nodes
  - **CLI Commands**:
    - `vel <file.vel>` - compile and display output
    - `vel build <file.vel>` - compile to native binary (Linux)
    - `vel asm <file.vel>` - generate assembly file
    - `vel tokens <file.vel>` - debug token stream
    - `vel version` - display version information

- **Examples**
  - `hello.vel` - Hello World program
  - `counter.vel` - Loops and conditionals demo
  - `functions.vel` - Function definitions and calls

- **Documentation**
  - Comprehensive README with quick start guide
  - Language reference documentation
  - Testing guide and procedures
  - Building instructions for Windows/Linux

### Technical Details
- **Language**: C++23
- **Target Platform**: x86-64 Linux (with cross-platform compiler)
- **ABI**: System V AMD64 calling convention
- **Assembly Backend**: NASM syntax
- **Compilation Model**: Single-pass to assembly
- **Memory Management**: Arena allocator for parsing phase

### Known Limitations
- No string concatenation (planned for v0.2.0)
- No arrays or collections
- No standard library
- No package manager
- No generics or templates
- Limited error diagnostics
- Linux assembly target only (binary compilation on Linux)

### Quality Metrics
- Zero compiler warnings (Wall, Wextra, Wpedantic)
- ~2,500 lines of C++23 code
- Compilation time: < 1 second
- Binary size: ~2 MB

---

## Version History Summary

| Version | Release Date | Status | Focus |
|---------|-------------|--------|-------|
| 0.1.0 | 2026-09-03 | Early Build | Core language & compiler infrastructure |
| 0.2.0 | Planned | - | String ops, arrays, diagnostics |
| 1.0.0 | Planned | - | Multi-target, stdlib, packages |

---

## Upgrading Guide

### From v0.0.x to v0.1.0

No previous release. Initial public release of Vel.

---

## Future Release Notes Template

For upcoming releases, maintainers should use this structure:

```markdown
## [X.Y.Z] - YYYY-MM-DD

### Added
- Brief description of new features

### Changed
- Behavior changes and improvements

### Fixed
- Bug fixes and corrections

### Removed
- Deprecated features removed

### Security
- Security-related changes if applicable

### Migration Guide
- Instructions for users upgrading from previous version
```

---

## Contributing to Changelog

When submitting pull requests:

1. Include a clear description of changes
2. Reference the issue number if applicable
3. Note in your PR if changelog update is needed
4. Follow the format above for consistency

---

## Discussion & Feedback

- **Suggest features**: [GitHub Discussions](https://github.com/zyrndotio/Vel/discussions)
- **Report bugs**: [GitHub Issues](https://github.com/zyrndotio/Vel/issues)
- **Propose changes**: Create a feature request issue

---

*Last updated: 2026-09-03*
