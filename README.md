# Vel Programming Language

<div align="center">

![Vel](assets/readme-header.png)

**Vel** — A fast, readable systems language designed for modern cross-platform desktop development.

> Readable like English. Fast like C. Built for the real world.

[![CI](https://github.com/zyrndotio/Vel/actions/workflows/ci.yml/badge.svg)](https://github.com/zyrndotio/Vel/actions/workflows/ci.yml)
[![GitHub License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](LICENSE)
[![Compiler Version](https://img.shields.io/badge/version-0.2.0-orange.svg)]()

</div>

---

## 🎯 What is Vel?

Vel is a systems programming language that prioritizes:

- **Readability** — Code that reads like what it does
- **Performance** — Compiles directly to native code with zero runtime overhead
- **Safety** — Immutable by default, explicit mutation, strong typing

Perfect for applications that need to be fast, maintainable, and reliable.

> **Platform note:** The compiler frontend is cross-platform. Native output currently targets x86-64 Linux, macOS, and Windows; Windows linking requires NASM plus MinGW-w64 or a compatible Win32 toolchain.

See the [platform support guide](docs/PLATFORM_SUPPORT.md), [CLI reference](docs/CLI_REFERENCE.md), and [testing guide](docs/TESTING.md) for details.

---

## ⚡ Quick Start

### Prerequisites
- **Windows/macOS/Linux**: `g++` (C++23) or compatible compiler
- **Native builds**: `cmake`, `nasm`, and a platform linker (`ld`/binutils on Linux, Apple `ld` on macOS, or MinGW-w64 on Windows)

### Build the Compiler

```bash
# Using g++ directly
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp

# Or using CMake
cmake -S . -B build && cmake --build build
```

### Create an installer/package with CPack

```bash
cmake -S . -B build
cmake --build build --config Release
cd build
cpack -C Release
```

On Windows, the existing Inno Setup installer is the primary distribution
path. CPack selects `productbuild` on macOS and DEB/RPM on Linux. CPack's
Windows WiX generator is an optional alternative and requires WiX separately.

### Run a Vel Program

```bash
# Compile and execute (shows output directly)
./vel examples/hello.vel

# Generate assembly and print it
./vel asm examples/hello.vel

# Compile to a native binary for the host target
./vel build examples/hello.vel
./hello  # Run the binary

# Emit Windows PE/COFF assembly from any host
./vel asm examples/hello.vel windows-x86_64 > hello-windows.asm

# Validate syntax without native tools (works on Windows/macOS/Linux)
./vel check examples/hello.vel

# Debug token stream
./vel tokens examples/hello.vel
```

## Windows Installer

Windows users can use the existing Inno Setup installer. After building the
Release executable, compile `vel_installer.iss` with Inno Setup to produce
`installer_output/VelSetup-x64.exe`:

```powershell
cmake -S . -B build
cmake --build build --config Release
ISCC.exe vel_installer.iss
```

The resulting installer is the recommended Windows distribution. The Windows CI job installs Inno Setup, compiles this script, and publishes `VelSetup-windows-x64` as a downloadable artifact. CPack's WiX generator can also produce an `.msi`, but requires WiX to be installed separately.

## Packaging with CPack

CPack is included with CMake and uses the native package generator selected by
the host platform. Build the project first, then run CPack from the build
directory:

```bash
cmake -S . -B build
cmake --build build --config Release
cd build
cpack -C Release
```

The generated package format depends on the platform:

- **Windows**: WiX installer (`.msi`), with WiX installed separately
- **macOS**: `productbuild` package (`.pkg`)
- **Linux**: Debian (`.deb`) and RPM (`.rpm`) packages when the generators are available

---

## 📚 Language Features

The parser and type checker currently support scalar values plus frontend validation for homogeneous arrays, array indexing, named structs, struct literals, field access, and string concatenation. Native aggregate layout and runtime string-buffer emission are the next compiler stage.

### Variables & Types

```vel
let name = "Vel";           // immutable
mut count = 0;              // mutable

let x: int = 42;            // explicit types
let ratio: float = 3.14;
let active: bool = true;
let message: str = "hello";
```

### Functions

```vel
fn add(a: int, b: int) -> int {
    return a + b;
}

fn greet(name: str) {
    print "Hello, ";
    print name;
    print "!";
}

let sum = add(5, 3);  // 8
greet("World");
```

### Control Flow

```vel
// Conditionals
if (x > 10) {
    print "big";
} elif (x == 10) {
    print "just right";
} else {
    print "small";
}

// Loops
mut i = 0;
while (i < 5) {
    print i;
    i = i + 1;
}

// Infinite loop with break and continue
loop {
    if (condition) {
        break;
    }
}

// `continue` is supported in while and loop bodies.

### Operators

| Category | Operators |
|----------|-----------|
| **Arithmetic** | `+`, `-`, `*`, `/`, `%` |
| **Comparison** | `==`, `!=`, `<`, `>`, `<=`, `>=` |
| **Logical** | `&&`, `\|\|`, `!` |
| **Assignment** | `=` |

### Comments

```vel
// Single-line comment

/* Multi-line
   block comment */
```

---

## 📁 Project Structure

```
Vel/
├── src/                 # Compiler source code (C++23)
│   ├── main.cpp        # Entry point and CLI
│   ├── tokenizer.hpp   # Lexical analysis
│   ├── parser.hpp      # Syntax analysis
│   ├── ast.hpp         # Abstract syntax tree
│   ├── codegen.hpp     # Code generation (x86-64)
│   └── arena.hpp       # Memory arena allocator
├── examples/           # Sample Vel programs
│   ├── hello.vel       # Hello World
│   ├── counter.vel     # Loops and conditionals
│   └── functions.vel   # Function definitions
├── docs/              # Documentation
├── CMakeLists.txt     # Build configuration
├── README.md          # This file
├── CHANGELOG.md       # Version history
├── tests/              # Cross-platform smoke tests
├── .github/workflows/  # Linux/macOS/Windows CI
├── LICENSE            # GPL-3.0
├── SECURITY.md        # Security reporting policy

```

---

## 🏗️ Architecture

The Vel compiler has a portable frontend (tokenizer, parser, syntax checking, and NASM emission) and an x86-64 Linux native backend. The frontend builds and runs on Windows, macOS, and Linux; native program execution currently requires Linux x86-64 plus NASM and GNU binutils. CI verifies the portable frontend on all three desktop operating systems.

```
Source Code (.vel)
    ↓
Tokenizer (Lexical Analysis)
    ↓
Parser (Syntax Analysis)
    ↓
AST Construction (with Arena Allocator)
    ↓
Code Generation (x86-64 Assembly)
    ↓
NASM Assembly (Assembler - Linux only)
    ↓
GNU ld Linking (Linker - Linux only)
    ↓
Native Binary Executable
```

### Design Highlights

- **Single-pass compilation** for fast iteration
- **Arena allocator** for efficient memory management during parsing
- **Header-only implementation** (no build dependencies)
- **No runtime** — compiles directly to syscalls
- **System V ABI** calling convention compliance

---

## 🎓 Examples

### 1. Hello World
```vel
print "Hello, World!";
```

### 2. Counter with Loop
```vel
mut count = 0;
let limit = 10;

while (count < limit) {
    print count;
    count = count + 1;
}
```

### 3. Functions
```vel
fn multiply(x: int, y: int) -> int {
    return x * y;
}

let result = multiply(6, 7);
print result;  // outputs: 42
```

See [examples/](examples/) for more programs.

---

## 🔧 Commands

```
vel <file.vel>          Compile to assembly and display output
vel build <file.vel>    Compile to native binary (Linux only)
vel asm <file.vel>      Generate assembly file (.asm)
vel tokens <file.vel>   Debug: print token stream
vel version             Display compiler version
```

---

## 📖 Documentation

- [Building & Installation](docs/BUILDING.md)
- [CLI Reference](docs/CLI_REFERENCE.md)
- [Language Reference](docs/LANGUAGE_REFERENCE.md)
- [Testing Guide](docs/TESTING.md)
- [Contributing Guidelines](CONTRIBUTING.md)
- [Changelog](CHANGELOG.md)
- [Future Roadmap](ROADMAP.md)

---

## 🛣️ Roadmap

### Current (v0.2.0) ✅
- [x] Variables (`let` / `mut`)
- [x] Basic types (`int`, `float`, `str`, `bool`)
- [x] Arithmetic & comparison operators
- [x] Functions with parameters and return values
- [x] Control flow (`if` / `elif` / `else`)
- [x] Loops (`while`, `loop`, `break`)
- [x] Print built-in function
- [x] Assembly code generation
- [x] Cross-platform targets and release packaging
- [x] Arrays, structs, and string concatenation in the parser/type checker
- [x] `vel new`, `vel run`, `vel doctor`, and `vel clean`

### Planned (v0.2.1) 🚀
- [ ] Native array and struct layout improvements
- [ ] String methods and bounds checks
- [ ] Better error messages with diagnostics
- [ ] Type inference improvements
- [ ] Module system basics

### Future (v1.0.0) 🔮
- [ ] ARM64 targets
- [ ] Standard library
- [ ] Package manager
- [ ] Optimization passes
- [ ] Debugging symbols
- [ ] Generics and polymorphism

See [ROADMAP.md](ROADMAP.md) for detailed feature proposals.

---

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for:

- How to build from source
- Testing procedures
- Code style guidelines
- Pull request process

---

## 📄 License

Vel is released under the [GNU General Public License v3.0](LICENSE).

---

## 🙋 Support

- **Language questions?** Check the [Language Reference](docs/LANGUAGE_REFERENCE.md)
- **Build or command questions?** See the [Build Guide](docs/BUILDING.md) or [CLI Reference](docs/CLI_REFERENCE.md)
- **Found a bug?** [Open an issue](https://github.com/zyrndotio/Vel/issues)
- **Want to contribute?** See [CONTRIBUTING.md](CONTRIBUTING.md)

---

## 📊 Project Stats

- **Language**: C++23
- **Lines of Code**: ~2,500
- **Compilation Time**: < 1 second
- **Compiler Size**: ~2 MB
- **Build System**: CMake (optional)

---

<div align="center">

Made for system programmers who value readability.

[GitHub](https://github.com/zyrndotio/Vel) · [Issues](https://github.com/zyrndotio/Vel/issues) · [Discussions](https://github.com/zyrndotio/Vel/discussions)

</div>
