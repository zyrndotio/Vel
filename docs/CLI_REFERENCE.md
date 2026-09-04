# CLI Reference

Command-line interface documentation for the Vel compiler.

---

## Basic Syntax

```
vel [COMMAND] [FILE]
```

---

## Commands

### Default: Compile and Display

```
vel <file.vel>
```

Tokenizes, parses, and generates assembly for a Vel source file. Displays the generated x86-64 assembly.

**Example:**
```bash
vel examples/hello.vel
```

**Output:**
```
section .text
global _start

; ... assembly code ...
```

### `new` — Create a Project

```
vel new <project>
```

Creates a starter project with `vel.toml`, `src/main.vel`, `tests/`, and a project README. The manifest declares the entry point and test directory.

### `run` — Compile and Run

```
vel run <file.vel|project>
```

Compiles and executes a Vel program for the host target. A project directory or `vel.toml` resolves its declared `entry` file.

### `check` — Validate Without Native Tools

```
vel check <file.vel|project>
```

Tokenizes and parses a Vel source file without assembling or linking it. When passed a project directory or `vel.toml`, it resolves the manifest's `entry` path first. This is the recommended validation command on Windows and macOS and is also useful in editor integrations.

### `test` — Check Vel Test Sources

```
vel test [path|project]
```

Recursively discovers `.vel` files under `tests/` when no path is provided, or under the supplied file/directory. When passed a project directory or `vel.toml`, it uses the manifest's `test` directory. Each source is tokenized, parsed, and type-checked. This portable first version validates source fixtures; runtime assertions, test filters, and test reporting are planned for the project test system.

### `build` — Compile to Binary

```
vel build <file.vel|project>
```

Compiles a Vel program to a native binary executable for the host target. A project directory or `vel.toml` resolves its declared `entry` file.

**Requirements:**
- NASM (assembler)
- GNU ld (linker)
- Linux x86-64 target

**Example:**
```bash
vel build examples/hello.vel
./hello
```

**Output Files:**
- `program.asm` - Temporary generated assembly, removed after a successful build
- `program.o` - Temporary object file, removed after a successful build
- `program` - Executable binary written beside the input `.vel` file

**On Windows/macOS:**

Windows x86-64 can use the PE/COFF backend when NASM and MinGW-w64 (or another compatible linker) are installed. macOS x86-64 can use the Mach-O backend; macOS ARM64 should select `vel asm ... macos-x86_64` for assembly output and cannot currently link a native ARM64 binary.

### `asm` — Generate Target Assembly

```
vel asm <file.vel> [target]
```

Generates assembly to standard output. The optional target is `linux-x86_64`, `macos-x86_64`, or `windows-x86_64`; when omitted, the host default is selected.

Generates assembly and writes it to standard output without running linking steps.

**Example:**
```bash
vel asm examples/counter.vel
vel asm examples/counter.vel > counter.asm  # Save the assembly
```

**Output:**
- Prints x86-64 NASM syntax to standard output.
- `linux-x86_64` emits ELF/Linux sections and syscalls.
- `macos-x86_64` emits Mach-O/Darwin sections and syscalls.
- `windows-x86_64` emits PE/COFF sections and Win32 API imports for native linking.

### `update` — Check or Download a Release

```
vel update
vel update --download
```

Queries the latest published Vel release, selects the host-compatible archive, and verifies the release asset’s SHA-256 digest after download. Without `--download`, it only reports status. With `--download`, it saves the verified archive in the current directory and does not replace the running compiler or make privileged changes.

### `doctor` — Inspect Tooling

```
vel doctor
```

Reports whether CMake, NASM, and the host native backend are available.

### `clean` — Remove Generated Artifacts

```
vel clean <file.vel>
```

Removes generated assembly, object, and executable artifacts associated with a source file.

### `tokens` — Debug Token Stream

```
vel tokens <file.vel>
```

Lexical analysis only. Displays all tokens from the source file with line and column information.

**Example:**
```bash
vel tokens examples/hello.vel
```

**Output:**
```
[4:1] `print`
[4:7] string literal = Hello, World!
[4:22] `;`
[5:1] end of file
```

**Useful for:**
- Debugging tokenizer issues
- Understanding lexical structure
- Verifying token stream

### `version` — Show Compiler Version

```
vel version
```

Displays compiler version and target information.

**Example:**
```bash
vel version
```

**Output:**
```
Vel 0.3.1
Frontend: portable C++23
Native backends: Linux x86-64, macOS x86-64, Windows x86-64
```

---

## Exit Codes

| Code | Meaning | Example |
|------|---------|---------|
| 0 | Success | Compilation succeeded |
| 1 | Compilation error | Syntax error, undefined variable |
| 2 | File not found | File doesn't exist |
| 3 | System error | NASM/ld not found |

---

## File Processing

### Input Files

- **Extension**: `.vel`
- **Encoding**: UTF-8
- **Required**: Must exist and be readable

```bash
vel program.vel        # OK
vel program            # Error: needs .vel extension
vel nonexistent.vel    # Error: file not found
```

### Output Files

| Command | Output Files | Location |
|---------|--------------|----------|
| Default | None (printed to stdout) | Console |
| `asm` | `<name>.asm` | Current directory |
| `build` | `.asm`, `.o`, executable | Current directory |
| `tokens` | None (printed to stdout) | Console |

---

## Examples

### Example 1: View Tokens
```bash
vel tokens examples/hello.vel
```

### Example 2: Generate Assembly
```bash
vel asm examples/counter.vel > counter.asm
cat counter.asm
```

### Example 3: Compile to Binary (Linux)
```bash
vel build examples/functions.vel
./functions
```

### Example 4: Check Syntax
```bash
# Will show errors if any
vel asm myprogram.vel 2>&1 | head -20
```

---

## Error Messages

### File Not Found
```
[Vel] Cannot open file: missing.vel
```

### File Extension Error
```
[Vel] Input file must have .vel extension
```

### NASM Not Found
```
[Vel] Assembling with NASM...
sh: nasm: command not found
```

**Solution**: Install NASM
```bash
sudo apt-get install nasm        # Linux
brew install nasm                # macOS
```

### Linker Error
```
[Vel] Linking with ld...
ld: cannot open output file 'program': Permission denied
```

**Solution**: Check file permissions or use different directory

---

## Workflow Examples

### Development Workflow

```bash
# 1. Edit source
vim myprogram.vel

# 2. Check tokens
vel tokens myprogram.vel

# 3. View assembly
vel asm myprogram.vel | less

# 4. Debug with assembly
vel asm myprogram.vel > myprogram.asm
objdump -d myprogram.asm

# 5. Compile when ready
vel build myprogram.vel

# 6. Run binary
./myprogram
```

### Testing Workflow

```bash
# Test all examples
for example in examples/*.vel; do
    echo "Testing $example"
    vel asm "$example" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "  ✓ Pass"
    else
        echo "  ✗ Fail"
    fi
done
```

### Continuous Integration

```bash
#!/bin/bash
# CI script
set -e

echo "Building compiler..."
g++ -std=c++23 -o vel src/main.cpp

echo "Testing examples..."
for file in examples/*.vel; do
    ./vel tokens "$file" > /dev/null
    ./vel asm "$file" > /dev/null
done

echo "All tests passed!"
```

---

## Tips & Tricks

### Redirect Output to File
```bash
vel asm program.vel > program.asm
vel tokens program.vel > program.tokens
```

### Check Specific Error
```bash
vel asm program.vel 2>&1 | grep -i error
```

### Compare Assembly
```bash
vel asm file1.vel > asm1.txt
vel asm file2.vel > asm2.txt
diff asm1.txt asm2.txt
```

### Time Compilation
```bash
time vel asm program.vel
```

### Verbose Output (when debugging)
Edit `main.cpp` to enable verbose mode:
```cpp
compile(vel_path, true);  // true = verbose
```

---

## Environment Variables

Currently none, but planned for future versions:

- `VEL_OPTIMIZATION` - Optimization level
- `VEL_TARGET` - Target platform
- `VEL_DEBUG` - Debug mode

---

## Scripting

### PowerShell Example
```powershell
Get-ChildItem examples/*.vel | ForEach-Object {
    Write-Host "Compiling $_"
    ./vel asm $_
}
```

### Bash Example
```bash
#!/bin/bash
for file in examples/*.vel; do
    echo "Processing $file"
    ./vel asm "$file"
done
```

### Python Example
```python
import subprocess
import os

for file in os.listdir('examples'):
    if file.endswith('.vel'):
        result = subprocess.run(['./vel', 'asm', f'examples/{file}'])
        print(f"Exit code: {result.returncode}")
```

---

## Troubleshooting

### Program Won't Run
```bash
# Check if file exists
ls -la program.vel

# Check syntax
vel tokens program.vel

# View full error
vel asm program.vel 2>&1
```

### Assembly Output is Confusing
```bash
# Generate to file for easier reading
vel asm program.vel > program.asm

# View with line numbers
cat -n program.asm | less
```

### Binary Won't Execute
```bash
# Check permissions
ls -l program

# Make executable (Linux)
chmod +x program

# Run with explicit path
./program
```

---

## Platform-Specific Notes

### Windows
- Use PowerShell or WSL for best results
- `.exe` extension added automatically
- NASM/ld only in WSL or Linux subsystem

### Linux
- All features fully supported
- Use `.o` extension for object files automatically
- NASM and binutils required for `build` command

### macOS
- Compiler runs natively
- Binary generation produces Linux binaries
- macOS target support planned for v1.0.0

---

## Related

- [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md)
- [TESTING.md](TESTING.md)

---

*Last Updated: 2026-09-03*
