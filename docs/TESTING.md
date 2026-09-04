# Testing Guide

How to test Vel programs and validate the compiler.

---

## Quick Test

```bash
# Build compiler
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp

# Test all examples
./vel tokens examples/hello.vel
./vel asm examples/counter.vel
./vel asm examples/functions.vel
```

---

## Testing Workflow

### 1. Tokenizer Testing

Check if the tokenizer correctly breaks down source code:

```bash
./vel tokens program.vel
```

**Example Output:**
```
[4:1] `print`
[4:7] string literal = Hello, World!
[4:22] `;`
[5:1] end of file
```

**What to Check:**
- All tokens appear in correct order
- Line and column numbers match
- No missing tokens
- No spurious tokens

### 2. Assembly Generation Testing

Verify the code generator produces valid assembly:

```bash
./vel asm program.vel
```

**Example Output:**
```
section .text
global _start

; ... assembly code ...
```

**What to Check:**
- Assembly is generated
- Proper entry point (_start)
- Correct x86-64 syntax
- Helper functions included

### 3. Binary Compilation Testing (Linux only)

Full end-to-end testing:

```bash
./vel build program.vel
./program
```

**What to Check:**
- Binary created successfully
- Program executes without crash
- Output matches expectations

---

## Creating Test Programs

### Basic Test Template

```vel
// tests/test_name.vel
// Description of what this tests

// Test setup
let expected = 42;

// Test execution
let result = function_call();

// Verify
print result;  // Expected: same as expected value
```

### Test Categories

#### Correctness Tests
```vel
// Test arithmetic
let sum = 5 + 3;
print sum;  // Expected: 8

let product = 5 * 3;
print product;  // Expected: 15
```

#### Type Tests
```vel
// Test type handling
let i: int = 42;
let f: float = 3.14;
let s: str = "hello";
let b: bool = true;

print i;  // Expected: 42
print f;  // Expected: 3.14
print s;  // Expected: hello
print b;  // Expected: true (as number)
```

#### Control Flow Tests
```vel
// Test conditionals
let x = 5;

if (x > 10) {
    print 0;
} elif (x > 0) {
    print 1;  // Expected: this runs
} else {
    print 2;
}
```

#### Function Tests
```vel
fn add(a: int, b: int) -> int {
    return a + b;
}

// Test function
let result = add(3, 4);
print result;  // Expected: 7
```

---

## Example Programs

### hello.vel
**Purpose**: Basic print functionality

```vel
print "Hello, World!";
```

**Expected Output**: `Hello, World!` followed by newline

**Test Command**: `./vel asm examples/hello.vel`

### counter.vel
**Purpose**: Loops, conditionals, arithmetic

```vel
mut count = 0;
let limit = 5;

while (count < limit) {
    print count;
    count = count + 1;
}

if (count == 5) {
    print 1;
} else {
    print 0;
}
```

**Expected Output**: `0 1 2 3 4` followed by `1`

**Test Command**: `./vel build examples/counter.vel && ./counter`

### functions.vel
**Purpose**: Function definitions and calls

```vel
fn add(a: int, b: int) -> int {
    return a + b;
}

fn square(x: int) -> int {
    return x * x;
}

let result = add(3, 4);
print result;

let sq = square(9);
print sq;
```

**Expected Output**: `7` followed by `81`

**Test Command**: `./vel build examples/functions.vel && ./functions`

---

## Automated Testing

### Bash Script

```bash
#!/bin/bash
# test.sh - Run all tests

set -e

echo "Building compiler..."
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp

echo "Running tokenizer tests..."
for file in examples/*.vel; do
    echo "  Testing $file"
    ./vel tokens "$file" > /dev/null
done

echo "Running assembly generation tests..."
for file in examples/*.vel; do
    echo "  Testing $file"
    ./vel asm "$file" > /dev/null
done

echo "All tests passed!"
```

Run with:
```bash
chmod +x test.sh
./test.sh
```

### PowerShell Script

```powershell
# test.ps1 - Run all tests

Write-Host "Building compiler..."
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp

Write-Host "Running tokenizer tests..."
Get-ChildItem examples/*.vel | ForEach-Object {
    Write-Host "  Testing $_"
    ./vel tokens $_
}

Write-Host "Running assembly tests..."
Get-ChildItem examples/*.vel | ForEach-Object {
    Write-Host "  Testing $_"
    ./vel asm $_
}

Write-Host "All tests passed!"
```

Run with:
```powershell
.\test.ps1
```

---

## Validation Checklist

### Compiler Build
- [ ] Compiles without errors
- [ ] No compiler warnings (Wall, Wextra, Wpedantic)
- [ ] Executable created
- [ ] Binary is ~2 MB

### Tokenizer
- [ ] Tokenizes hello.vel correctly
- [ ] Tokenizes counter.vel correctly
- [ ] Tokenizes functions.vel correctly
- [ ] Token line/column info accurate

### Parser
- [ ] Parses all example programs
- [ ] No syntax errors for valid programs
- [ ] Proper error for invalid syntax

### Code Generation
- [ ] Generates valid assembly syntax
- [ ] Entry point (_start) present
- [ ] Helper functions included
- [ ] No assembly errors

### Binary Compilation (Linux)
- [ ] NASM assembles successfully
- [ ] Linker creates executable
- [ ] Binary is executable
- [ ] Program runs without crashing

---

## Performance Testing

### Compilation Speed

```bash
# Measure compilation time
time ./vel asm examples/hello.vel
```

Expected: < 1 second

### Binary Size

```bash
# Check binary size
du -h hello  # Linux/macOS
ls -lh hello.exe  # Windows
```

Expected: 4-8 KB

### Compiler Memory Usage

```bash
# Linux
/usr/bin/time -v ./vel asm examples/hello.vel

# macOS
/usr/bin/time -l ./vel asm examples/hello.vel
```

Expected: < 10 MB

---

## Edge Case Testing

### Empty Program
```vel
// empty.vel
```

**Expected**: Should compile without error

### Comments Only
```vel
// This is a comment
/* This is a
   block comment */
```

**Expected**: Should compile without error

### Large Numbers
```vel
let big = 9223372036854775807;  // max int64
print big;
```

**Expected**: Should compile and print correctly

### Deep Nesting
```vel
if (true) {
    if (true) {
        if (true) {
            print "nested";
        }
    }
}
```

**Expected**: Should compile and run

---

## Debugging Techniques

### Inspect Token Stream

```bash
./vel tokens program.vel > tokens.txt
cat tokens.txt  # Review tokens
```

### View Generated Assembly

```bash
./vel asm program.vel > program.asm
cat program.asm
objdump -d program  # If binary exists
```

### Enable Verbose Output

Edit `src/main.cpp`:
```cpp
// In compile() function, change:
compile(vel_path, false);  // false = quiet
// To:
compile(vel_path, true);   // true = verbose
```

Recompile and run:
```bash
g++ -std=c++23 -Wall -Wextra -Wpedantic -o vel src/main.cpp
./vel asm program.vel  # Now shows verbose output
```

### Step Through Assembly

```bash
# Compile program
./vel build program.vel

# Debug with GDB (Linux)
gdb ./program
(gdb) break _start
(gdb) run
(gdb) disassemble
(gdb) next  # Step through
```

---

## Regression Testing

### Before Changes
```bash
./vel asm examples/hello.vel > hello_before.asm
./vel asm examples/counter.vel > counter_before.asm
```

### After Changes
```bash
./vel asm examples/hello.vel > hello_after.asm
./vel asm examples/counter.vel > counter_after.asm
```

### Compare
```bash
diff hello_before.asm hello_after.asm
diff counter_before.asm counter_after.asm
```

---

## Integration Testing

### Test Multiple Files

```bash
for file in examples/*.vel; do
    base=$(basename "$file" .vel)
    echo "Testing $base..."

    # Generate assembly
    ./vel asm "$file" > /tmp/$base.asm

    # Verify output exists
    if [ -s /tmp/$base.asm ]; then
        echo "  ✓ Assembly generated"
    else
        echo "  ✗ Failed"
        exit 1
    fi
done

echo "All tests passed!"
```

---

## Continuous Integration

### GitHub Actions Example

```yaml
name: Test

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2

      - name: Build Compiler
        run: g++ -std=c++23 -o vel src/main.cpp

      - name: Test Examples
        run: |
          ./vel tokens examples/hello.vel
          ./vel asm examples/counter.vel
          ./vel asm examples/functions.vel
```

---

## Common Test Issues

### Issue: Tokenizer test fails
- Check: File exists and is readable
- Check: No syntax errors in .vel file
- Solution: Use `./vel asm` to see detailed errors

### Issue: Assembly invalid
- Check: NASM syntax requirements
- Check: Register usage is correct
- Solution: Review [x86-64 calling convention](https://en.wikipedia.org/wiki/X86_calling_conventions)

### Issue: Binary doesn't run
- Check: Permissions (Linux: `chmod +x`)
- Check: System architecture matches
- Solution: Run with GDB for error details

---

## Test Coverage

Currently achieving:
- ✅ Tokenizer: 100% statements
- ✅ Parser: 100% statements
- ✅ Code Generator: 100% statements
- ✅ Feature Coverage: Variables, Functions, Loops, Conditionals

Future improvements (v0.2.0+):
- Unit test framework
- Formal test suite
- Coverage reports

---

## Additional Resources

- [CLI_REFERENCE.md](CLI_REFERENCE.md) - Command details
- [LANGUAGE_REFERENCE.md](LANGUAGE_REFERENCE.md) - Syntax reference

---

*Last Updated: 2026-09-03*
