# Changelog

All notable changes to the Vel programming language will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [0.3.0] - 2026-09-04

### Added
- Added the initial native string allocation ABI and a large-concatenation runtime test.
- Added native array headers, read bounds checks, dynamic scalar-array storage, indexed writes, and append growth.
- Added resizing and mutation semantics for nested aggregate arrays.
- Completed struct layout, nested aggregate access, and mutable field writes.
- Added string methods, explicit length metadata, and improved diagnostics.
- Added aggregate function parameters and return values.
- Defined ownership, lifetime, and allocation-failure semantics for aggregate runtime values.
- Added cross-platform native execution validation.

## [0.2.0] - 2026-09-04

### Added
- Added `vel new`, `vel run`, `vel doctor`, and `vel clean` project/developer commands.
- Added a Vel logo asset and included it in packages and the Windows installer.
- Added native pointer-based array literals/indexing and struct field-offset access for static aggregate layouts.
- Added runtime string concatenation with persistent buffer allocation and string-aware printing.
- Added complex nested aggregate fixtures covering multidimensional arrays, nested structs, empty strings, and repeated concatenation.
- Added a tag-triggered release workflow for Linux, macOS, and Windows artifacts, including the Inno Setup installer.

### Changed
- Updated the roadmap and documentation for the v0.2.0 frontend and native-runtime milestone.

## [0.1.1] - 2026-09-04

### Added
- Added parser/type-checker validation for function argument count and types, return values, variable types, assignment mutability, conditions, and loop-control placement.
- Added macOS x86-64 Mach-O assembly emission and target selection via `vel asm <file.vel> macos-x86_64`.
- Added comprehensive nested-loop `continue` fixtures and negative type-checker tests.
- Added repository release-readiness files, contributor templates, security guidance, Makefile helpers, and guarded CPack RPM generation.

### Fixed
- Corrected the Ubuntu CI hello-world assertion to match the checked-in example output.
- Prevented failed tests from leaving generated binaries and temporary assembly files behind.

## [Unreleased]

### Added
- Added a portable `vel check` command for syntax validation without NASM or a host linker.
- Added CTest smoke coverage and GitHub Actions builds for Linux, macOS, and Windows.
- Added a clearer runtime capability report to `vel version`.
- Added parser/type-checker support for homogeneous arrays, array indexing, named structs, struct literals, field access, and string concatenation.
- Added Windows x86-64 PE/COFF assembly emission and Win32 API imports for native linking.
- Added Windows CI coverage for PE/COFF assembly and Inno Setup installer generation.

### Changed
- Native compilation now writes generated artifacts beside the input `.vel` file and safely quotes paths.
- Native compilation now selects Linux ELF, macOS Mach-O, or Windows PE/COFF based on the host target.
- Implemented `continue` for `while` and `loop`, including stack cleanup for skipped scopes.
- Aggregate syntax and type checking are currently frontend-only; native array/struct layout and runtime string-buffer emission remain planned.

### Planned
- Native array and struct storage/layout code generation
- Native string concatenation buffers and string methods
- Better error messages with line/column diagnostics
- Full recursive type checking for aggregate function arguments and returns
- Module system basics
- ARM64 targets and Windows runtime validation across MinGW/MSVC toolchains

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
