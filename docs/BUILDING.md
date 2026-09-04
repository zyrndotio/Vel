# Building & Installation

How to build the Vel compiler from source on your system.

---

## Requirements

### Minimum
- **Compiler**: GCC 11+ or Clang 14+ with C++23 support
- **RAM**: 512 MB
- **Disk**: 50 MB

### Recommended
- **Compiler**: GCC 12+ or Clang 15+
- **RAM**: 2 GB
- **Disk**: 200 MB (with build artifacts)

### For Full Compilation (Linux)
Additional tools needed for native binary generation:
- **NASM** (Netwide Assembler) - for x86-64 assembly
- **GNU binutils** - includes `ld` linker
- **CMake** (optional, v3.22+) - for build system

---

## Platform-Specific Instructions

### Windows

#### Step 1: Install Tools
```powershell
# If you have MSYS2 or MinGW installed
# Ensure g++ is in your PATH
g++ --version

# Should output: g++ (version) ...
```

#### Step 2: Clone/Download
```powershell
# Clone from GitHub
git clone https://github.com/zyrndotio/Vel.git
cd Vel
```

#### Step 3: Compile
```powershell
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp
```

#### Step 4: Verify
```powershell
./vel version
# Should output: Vel 0.1.0 — Early Build
```

### Linux (Ubuntu/Debian)

#### Step 1: Install Dependencies
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake nasm binutils
```

Verify installation:
```bash
g++ --version
cmake --version
nasm --version
ld --version
```

#### Step 2: Clone Repository
```bash
git clone https://github.com/zyrndotio/Vel.git
cd Vel
```

#### Step 3: Build (Option A - Direct)
```bash
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp
```

#### Step 3: Build (Option B - CMake)
```bash
cmake -S . -B build
cmake --build build
# Compiler binary at: build/vel
```

#### Step 4: Verify
```bash
./vel version
```

### Linux (Fedora/RHEL)

```bash
# Install dependencies
sudo dnf install -y gcc gcc-c++ cmake nasm binutils

# Build
git clone https://github.com/zyrndotio/Vel.git
cd Vel
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp
```

### Linux (Arch)

```bash
# Install dependencies
sudo pacman -S base-devel cmake nasm binutils

# Build
git clone https://github.com/zyrndotio/Vel.git
cd Vel
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp
```

### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# For full compilation support, install NASM and binutils
brew install nasm binutils

# Clone and build
git clone https://github.com/zyrndotio/Vel.git
cd Vel
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp
```

**Note**: Native binary compilation on macOS generates Linux binaries. macOS target support is planned for v1.0.0.

---

## Build Verification

After building, verify everything works:

```bash
# Test 1: Check version
./vel version

# Test 2: Tokenize a program
./vel tokens examples/hello.vel

# Test 3: Generate assembly
./vel asm examples/hello.vel

# Test 4: Compile to binary (Linux only)
./vel build examples/hello.vel
./hello  # Run the generated binary
```

All tests should complete without errors.

---

## Build Options

### Compile Flags

```bash
# Default (recommended)
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp

# With optimization
g++ -std=c++23 -O2 -Wall -Wextra -Wpedantic -o vel src/main.cpp

# Debug build (with symbols)
g++ -std=c++23 -g -Wall -Wextra -Wpedantic -o vel src/main.cpp

# No warnings (not recommended)
g++ -std=c++23 -o vel src/main.cpp
```

### Using Different Compilers

```bash
# Using Clang
clang++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp

# Using Intel Compiler
icpx -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp
```

---

## Troubleshooting

### Issue: `g++: command not found`

**Solution**: Install a C++23-compatible compiler

- **Windows**: Install MinGW or MSYS2
- **Linux**: `sudo apt-get install build-essential` (Ubuntu) or equivalent
- **macOS**: `xcode-select --install`

### Issue: `error: unrecognized command line option '-std=c++23'`

**Solution**: Upgrade your compiler

```bash
# Check compiler version
g++ --version

# GCC: Requires 11.0+
# Clang: Requires 14.0+
# MSVC: Requires Visual Studio 2022 or newer
```

### Issue: `NASM not found` when using `vel build`

**Solution**: Install NASM

- **Linux**: `sudo apt-get install nasm`
- **macOS**: `brew install nasm`
- **Windows**: Download from [nasm.us](https://www.nasm.us/)

### Issue: Compilation takes too long

**Solution**: Use optimized build

```bash
# Faster linking with LTO disabled
g++ -std=c++23 -O2 -Wall -Wextra -Wpedantic -fno-lto -o vel src/main.cpp
```

---

## Installation Options

### Option 1: Local Build (Recommended for Development)
```bash
git clone https://github.com/zyrndotio/Vel.git
cd Vel
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp
./vel examples/hello.vel
```

### Option 2: System-Wide Installation (Linux)
```bash
# Build
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp

# Install to system path
sudo cp vel /usr/local/bin/

# Verify
which vel
vel version
```

### Option 3: CMake Installation (Linux)
```bash
cmake -S . -B build
cmake --build build
sudo cmake --install build
```

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

For a local install without creating a package, run `cmake --install build`
(or `cmake --install build --config Release` for a multi-configuration generator).

---

## Uninstallation

### Removing Local Build
```bash
# Just delete the directory
rm -rf Vel/
# Or the executable
rm ./vel
```

### Removing System-Wide Installation
```bash
sudo rm /usr/local/bin/vel
```

---

## Next Steps

After building:

1. **Run examples**: `./vel examples/hello.vel`
2. **Read the language guide**: See [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md)
3. **Review compiler commands**: See [CLI_REFERENCE.md](CLI_REFERENCE.md)
4. **Test your setup**: See [TESTING.md](TESTING.md)

---

## Build System Details

### Project Structure
```
Vel/
├── src/
│   ├── main.cpp          # Compiler entry point
│   ├── tokenizer.hpp     # Lexer
│   ├── parser.hpp        # Syntax analyzer
│   ├── ast.hpp          # AST definitions
│   ├── codegen.hpp      # Code generator
│   └── arena.hpp        # Memory allocator
├── examples/            # Test programs
├── CMakeLists.txt       # CMake configuration
└── README.md
```

### Compilation Process
```
Source files (.hpp, .cpp)
    ↓
C++ Compiler (g++/clang++)
    ↓
Object files (.o)
    ↓
Linker
    ↓
Executable (vel)
```

### CMake Configuration
- **Standard**: C++23
- **Flags**: -Wall -Wextra -Wpedantic
- **Output**: Single executable (`vel` or `vel.exe`)

---

## Performance

### Build Time
- Direct compilation: ~2-5 seconds
- CMake: ~1-2 seconds (with cached config)
- Clean rebuild: ~5-8 seconds

### Binary Size
- Release: ~2 MB
- Debug: ~5 MB
- Stripped: ~1 MB

---

## Version Compatibility

| GCC Version | Status |
|------------|--------|
| 10.x | ❌ No C++23 |
| 11.x | ✅ Works |
| 12.x | ✅ Recommended |
| 13.x+ | ✅ Works |

| Clang Version | Status |
|-------------|--------|
| 13.x | ❌ No C++23 |
| 14.x | ✅ Works |
| 15.x+ | ✅ Recommended |

| MSVC Version | Status |
|-------------|--------|
| VS 2019 | ❌ Limited C++23 |
| VS 2022 | ✅ Works |

---

## Environmental Variables

### PATH Configuration
For system-wide access, add Vel to PATH:

```bash
# Linux/macOS
export PATH="$PATH:/path/to/Vel"

# Windows (PowerShell)
$env:PATH += ";C:\path\to\Vel"
```

---

## Getting Help

- **Build issues**: Check [Troubleshooting](#troubleshooting) section
- **Installation help**: [GitHub Issues](https://github.com/zyrndotio/Vel/issues)
- **Questions**: [GitHub Discussions](https://github.com/zyrndotio/Vel/discussions)

---

<div align="center">

**Next**: [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md) · [CLI_REFERENCE.md](CLI_REFERENCE.md)

</div>

---

*Last Updated: 2026-09-03*
