# Contributing to Vel

Thank you for your interest in contributing to Vel! We welcome contributions from the community, whether it's bug reports, feature suggestions, documentation improvements, or code contributions.

---

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [Development Setup](#development-setup)
4. [Making Changes](#making-changes)
5. [Submitting Changes](#submitting-changes)
6. [Style Guidelines](#style-guidelines)
7. [Future Improvements](#future-improvements)
8. [Getting Help](#getting-help)

---

## Code of Conduct

This project adheres to the Contributor Covenant Code of Conduct. By participating, you are expected to uphold this code. Please report unacceptable behavior to the maintainers.

---

## Getting Started

### Prerequisites

- **Compiler**: GCC/Clang with C++23 support
- **Build Tools**: CMake 3.22+ (optional, g++ works directly)
- **Linux Users**: NASM and binutils (ld) for full native compilation
- **Version Control**: Git

### Fork and Clone

```bash
# Fork the repository on GitHub

# Clone your fork
git clone https://github.com/YOUR-USERNAME/Vel.git
cd Vel

# Add upstream remote
git remote add upstream https://github.com/zyrndotio/Vel.git
```

---

## Development Setup

### Build from Source

#### Option 1: Direct Compilation (All Platforms)
```bash
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp
```

#### Option 2: CMake (Recommended for Linux)
```bash
cmake -S . -B build
cmake --build build
./build/vel examples/hello.vel
```

### Testing

```bash
# Test tokenizer
./vel tokens examples/hello.vel

# Test assembly generation
./vel asm examples/hello.vel

# Test all examples (Linux)
./vel build examples/hello.vel
./hello
```

See [docs/TESTING.md](docs/TESTING.md) for comprehensive testing guide.

---

## Making Changes

### Create a Branch

```bash
git checkout -b feature/your-feature-name
# or
git checkout -b fix/issue-number
```

Use descriptive branch names:
- `feature/string-concatenation`
- `fix/parser-crash`
- `docs/improve-readme`
- `refactor/arena-allocator`

### Code Style Guidelines

#### C++23 Standards

- Use modern C++ features (auto, constexpr, templates)
- Prefer standard library utilities over raw pointers
- Use header-only implementation pattern
- Include proper error handling

#### Naming Conventions

```cpp
// Classes and types: PascalCase
class Parser { };
struct Token { };

// Functions and variables: snake_case
void parse_expression() { }
int token_count = 0;

// Constants: UPPER_SNAKE_CASE
const int MAX_TOKENS = 1000;

// Private members: _leading_underscore
class Lexer {
    int _current_pos = 0;
};
```

#### Formatting

- Use 4-space indentation
- Maximum line length: 100 characters
- One statement per line
- Opening brace on same line (K&R style)
- Comments above code blocks explaining intent

```cpp
// Example formatting
void process_tokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        // Process each token according to its type
        if (token.type == TT::Ident) {
            handle_identifier(token);
        }
    }
}
```

#### Documentation

- Add comments for complex algorithms
- Document public functions with brief descriptions
- Include examples in header comments
- Explain "why" not just "what"

```cpp
// Parses a function definition and returns the AST node
// Expected: 'fn' keyword already consumed
// Returns nullptr if parsing fails
std::unique_ptr<FnDef> parse_function(Arena& arena) {
    // ...
}
```

### Commit Messages

Write clear, descriptive commit messages:

```
Short summary (50 chars or less)

More detailed explanation of the change if needed.
Explain what and why, not how.

- Point 1
- Point 2

Fixes #123
```

Good examples:
- `feat: add string concatenation operator`
- `fix: parser crash on unclosed braces`
- `docs: improve language reference examples`
- `refactor: simplify tokenizer state machine`

---

## Submitting Changes

### Pull Request Process

1. **Update your branch** with latest upstream:
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```

3. **Create a Pull Request** on GitHub with:
   - Clear title and description
   - Reference to related issues (`Fixes #123`)
   - Screenshots/examples if applicable
   - Link to testing performed

4. **Address review feedback**:
   - Respond to comments
   - Make requested changes
   - Push updates (don't force-push after review starts)

5. **Merge** once approved and CI passes

### PR Checklist

- [ ] Code follows style guidelines
- [ ] Comments added for complex logic
- [ ] Tests pass locally
- [ ] No compiler warnings (Wall, Wextra, Wpedantic)
- [ ] Documentation updated if needed
- [ ] Commit messages are clear
- [ ] No .exe or build artifacts committed

---

## Style Guidelines

### What We Value

✅ **DO:**
- Write readable, self-documenting code
- Add tests for new features
- Comment complex algorithms
- Keep functions focused and small
- Use meaningful variable names
- Write helpful commit messages
- Document edge cases

❌ **DON'T:**
- Commit large refactors without discussion
- Mix formatting changes with logic changes
- Push directly to main
- Ignore compiler warnings
- Write cryptic single-letter variables
- Leave debug code in commits

---

## Future Improvements

### High Priority (v0.2.0)

#### 1. String Operations
- **String concatenation** with `+` operator
- **String methods**: `.len()`, `.substring()`
- **String interpolation**: `"Hello {name}"`
- **Character types**: `char` type support
- **Escape sequences**: `\n`, `\t`, `\\`, `\"`

```vel
// Future syntax
let greeting = "Hello" + " " + "World";
let msg = "Count: {count}";
let escaped = "Line 1\nLine 2";
```

#### 2. Array Types
- **Array literals**: `[1, 2, 3]`
- **Array indexing**: `arr[0]`
- **Array length**: `arr.len()`
- **Array slicing**: `arr[1:3]`
- **Type annotations**: `let nums: [int] = [1, 2, 3]`

```vel
// Future syntax
let nums: [int] = [1, 2, 3, 4, 5];
print nums[0];           // 1
print nums.len();        // 5
let slice = nums[1:3];   // [2, 3]
```

#### 3. Error Diagnostics
- **Better error messages** with context
- **Line and column numbers** in errors
- **Error recovery** to continue parsing
- **Warning system** for deprecations
- **Suggestion system** for common mistakes

```
Error [E001]: Undefined variable 'x'
  --> examples/test.vel:5:10
   |
 5 |     print x;
   |           ^ undefined variable
   |
   = Help: Did you mean 'count'?
```

#### 4. Type System Improvements
- **Type inference** for variables
- **Automatic type coercion** rules
- **Union types** (future)
- **Generic types** (future)

```vel
// Future syntax - type inference
let x = 42;           // inferred as int
let y = 3.14;         // inferred as float
let z = "hello";      // inferred as str
```

### Medium Priority (v0.3.0 - v0.4.0)

#### 5. Module System
- **Module declarations**: `module math`
- **Import statements**: `use std::io`
- **Public/private visibility**: `pub fn add()`
- **File organization**: Multi-file programs

#### 6. Advanced Features
- **Structs and records**: User-defined types
- **Enums**: Tagged unions
- **Pattern matching**: `match` statements
- **Generics**: Parametric types
- **Traits**: Interface definitions

#### 7. Compiler Optimizations
- **Constant folding**: Evaluate constants at compile-time
- **Dead code elimination**: Remove unused code
- **Inline optimization**: Inline small functions
- **Loop unrolling**: Optimize tight loops
- **Register allocation**: Better x86 code

#### 8. Standard Library
- **I/O operations**: File reading/writing
- **Math functions**: `sqrt()`, `pow()`, trigonometry
- **String utilities**: Parsing, formatting
- **Collections**: Vectors, hash maps
- **System calls**: Process management

### Long Term (v1.0.0)

#### 9. Multi-Target Support
- **Windows/PE**: Native Windows binaries
- **macOS/Mach-O**: Apple platform support
- **ARM64**: Apple Silicon and ARM systems
- **WebAssembly**: Browser compilation

#### 10. Developer Experience
- **Language Server Protocol (LSP)**: IDE integration
- **Debugger support**: GDB/LLDB compatibility
- **Build system**: Package manager (`vpm` - Vel Package Manager)
- **REPL**: Interactive shell
- **Documentation generator**: Auto-generate docs

#### 11. Performance & Analysis
- **Profiler**: Performance analysis
- **Memory analyzer**: Leak detection
- **Benchmarking tools**: Performance comparison
- **Coverage reports**: Test coverage metrics

#### 12. Community Features
- **Package registry**: Centralized package repository
- **Package manager**: Dependency management
- **Build profiles**: Debug/Release configurations
- **Template projects**: Project scaffolding

---

## Area-Specific Guidelines

### Working on the Tokenizer

The tokenizer converts source code into a token stream. When modifying:

1. Add new token types to the `TT` enum in `tokenizer.hpp`
2. Update the tokenization logic in the `tokenize()` method
3. Add test cases in examples
4. Update documentation

```cpp
enum class TT {
    // Add new token type here
    YourToken,
    // ...
};
```

### Working on the Parser

The parser builds the Abstract Syntax Tree (AST). Guidelines:

1. Add new AST node types to `ast.hpp`
2. Implement parse methods following the pattern
3. Update error handling
4. Test with sample programs

```cpp
// Pattern: parse_something() methods
std::unique_ptr<Statement> parse_your_statement() {
    // Implementation
}
```

### Working on Code Generation

The code generator produces x86-64 assembly. When contributing:

1. Understand x86-64 calling conventions
2. Test generated assembly
3. Verify syscalls are correct
4. Document assembly output format

### Working on Documentation

Documentation improvements are always welcome:

1. Keep language simple and clear
2. Include code examples
3. Update table of contents
4. Check links are working
5. Spell-check content

---

## Testing Your Changes

### Unit Testing Approach

Create `.vel` test files in `examples/` directory:

```vel
// examples/test_feature.vel
// Tests your new feature

// Test case 1
let result = your_feature();
print result;  // Expected output

// Test case 2
let other = your_feature(42);
print other;
```

Run with:
```bash
./vel asm examples/test_feature.vel > output.asm
./vel build examples/test_feature.vel
./test_feature
```

### Manual Testing Checklist

- [ ] Code compiles without warnings
- [ ] All examples still work
- [ ] New feature works as expected
- [ ] Error cases handled gracefully
- [ ] Documentation updated
- [ ] No performance regressions

---

## Getting Help

### Questions?

- **GitHub Issues**: Ask questions via [issues](https://github.com/zyrndotio/Vel/issues)
- **GitHub Discussions**: Chat in [discussions](https://github.com/zyrndotio/Vel/discussions)
- **Documentation**: Check [docs/](docs/) folder first

### Reporting Bugs

Include:
1. Minimal reproducible example
2. Expected vs actual behavior
3. System information (OS, compiler version)
4. Compiler output/error messages
5. Steps to reproduce

---

## Recognition

Contributors are recognized in:
- [README.md](README.md) contributors section
- GitHub contributor graph
- Release notes for significant contributions

---

## License

By contributing to Vel, you agree that your contributions will be licensed under the GNU General Public License v3.0.

---

## Thank You! 🙏

Your contributions help make Vel better for everyone. We appreciate your time and effort!

---

*For questions about these guidelines, please open an issue or discussion.*
