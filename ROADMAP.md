# Vel Roadmap

This document outlines the planned features and improvements for the Vel programming language.

**Current Version**: 0.1.0 (Early Build)
**Target Version**: 1.0.0 (Full Release)

---

## Release Timeline

```
┌─────────────────────────────────────────────────────────────┐
│  v0.1.0 ───→ v0.2.0 ───→ v0.3.0 ───→ v0.4.0 ───→ v1.0.0  │
│  (Current)   (6 months)  (6 months)  (6 months)  (12 months)│
│  Early       Core Lang.  Modules     Advanced   Production  │
│  Build       Features    System      Features   Ready       │
└─────────────────────────────────────────────────────────────┘
```

---

## v0.1.0 — Early Build ✅

**Status**: Released
**Date**: September 2026

### Completed Features ✓
- [x] Tokenizer with full token types
- [x] Recursive descent parser
- [x] AST construction with arena allocator
- [x] x86-64 assembly code generation
- [x] Basic type system (int, float, str, bool)
- [x] Variables (let/mut)
- [x] Functions with parameters and returns
- [x] Control flow (if/elif/else, while, loop, break)
- [x] Arithmetic and comparison operators
- [x] Print built-in function
- [x] CLI with multiple commands

### Known Limitations ⚠️
- No string concatenation
- No arrays or collections
- Limited error messages
- Linux assembly target only
- Single file programs only

---

## v0.2.0 — String & Array Essentials 🎯

**Target Date**: Q2 2027
**Estimated Features**: 6-8 weeks
**Focus**: Core data structures and operations

### Major Features

#### 1. String Operations
- [x] Plan: String concatenation with `+`
- [x] Plan: String comparison (==, !=, etc.)
- [ ] Implement string concatenation
- [ ] Add string escape sequences (\n, \t, etc.)
- [ ] Implement string methods (.len(), .substr())
- [ ] Add character type support
- **Priority**: High
- **Difficulty**: Medium
- **Estimated Time**: 1-2 weeks

```vel
// Example v0.2.0 feature
let greeting = "Hello" + " " + "World";
print greeting;  // "Hello World"
```

#### 2. Array Types
- [ ] Array literal syntax `[1, 2, 3]`
- [ ] Array indexing `arr[0]`
- [ ] Array length property `.len()`
- [ ] Array iteration patterns
- [ ] Bounds checking
- **Priority**: High
- **Difficulty**: High
- **Estimated Time**: 2-3 weeks

```vel
// Example v0.2.0 feature
let numbers: [int] = [1, 2, 3, 4, 5];
print numbers[0];      // 1
print numbers.len();   // 5
```

#### 3. Better Error Messages
- [ ] Line/column tracking in errors
- [ ] Error context with code snippets
- [ ] Helpful suggestions
- [ ] Warning system for deprecations
- **Priority**: Medium
- **Difficulty**: Medium
- **Estimated Time**: 1 week

```
Error [E001]: Undefined variable 'count'
  --> examples/test.vel:5:12
   |
 5 |     print count;
   |           ^^^^^ undefined variable
   |
   = Hint: Did you mean 'total'?
```

#### 4. Type Inference
- [ ] Local variable type inference
- [ ] Function return type inference
- [ ] Type constraint checking
- **Priority**: Medium
- **Difficulty**: High
- **Estimated Time**: 1-2 weeks

```vel
// Example: type inference
let x = 42;           // inferred as int
let y = 3.14;         // inferred as float
let name = "Vel";     // inferred as str
```

### Compiler Improvements
- [ ] Better code generation for arrays
- [ ] Optimize string operations
- [ ] Improve error recovery

### Documentation
- [ ] String reference guide
- [ ] Array tutorial
- [ ] Migration guide from v0.1.0

### Testing
- [ ] Comprehensive string tests
- [ ] Array boundary tests
- [ ] Error message validation

---

## v0.3.0 — Module System & Intermediate Features

**Target Date**: Q4 2027
**Estimated Scope**: 8-10 weeks

### Major Features

#### 1. Module System
- [ ] Module declarations
- [ ] Import/use statements
- [ ] Public/private visibility
- [ ] Module namespace resolution
- **Priority**: High
- **Difficulty**: Very High

```vel
// main.vel
use math::add;
use geometry::{Circle, square_area};

let result = add(3, 4);
```

#### 2. Structs and Records
- [ ] Struct definitions
- [ ] Field access
- [ ] Method definitions
- [ ] Constructor patterns
- **Priority**: High
- **Difficulty**: High

```vel
// Future syntax
struct Point {
    x: int,
    y: int
}

let p = Point { x: 10, y: 20 };
print p.x;
```

#### 3. Pattern Matching
- [ ] Match expressions
- [ ] Destructuring
- [ ] Guard clauses
- **Priority**: Medium
- **Difficulty**: High

```vel
// Future syntax
match value {
    0 => { print "zero"; }
    1..=10 => { print "small"; }
    _ => { print "large"; }
}
```

#### 4. Range Types
- [ ] Range literals `1..10`
- [ ] Inclusive ranges `1..=10`
- [ ] For-in loops with ranges
- **Priority**: Medium
- **Difficulty**: Low

```vel
// Future syntax
for i in 1..10 {
    print i;
}
```

### Compiler Features
- [ ] Multi-file compilation
- [ ] Symbol table across files
- [ ] Linker support

### Quality Improvements
- [ ] Compiler optimization passes
- [ ] Better register allocation

---

## v0.4.0 — Advanced Features & Optimization

**Target Date**: Q2 2028
**Estimated Scope**: 10 weeks

### Major Features

#### 1. Generics
- [ ] Generic type parameters
- [ ] Generic functions
- [ ] Generic structs
- [ ] Type constraints
- **Difficulty**: Very High

```vel
// Future syntax
fn get_first<T>(arr: [T]) -> T {
    return arr[0];
}
```

#### 2. Traits & Interfaces
- [ ] Trait definitions
- [ ] Trait implementations
- [ ] Trait bounds
- **Difficulty**: Very High

```vel
// Future syntax
trait Drawable {
    fn draw();
}

impl Drawable for Circle {
    fn draw() { ... }
}
```

#### 3. Enums & Sum Types
- [ ] Enum definitions
- [ ] Enum variants
- [ ] Associated data
- [ ] Enum pattern matching
- **Difficulty**: High

```vel
// Future syntax
enum Result<T, E> {
    Ok(T),
    Err(E)
}
```

#### 4. Advanced Operators
- [ ] Custom operator definitions
- [ ] Operator overloading
- [ ] Method call chaining
- **Difficulty**: Medium

### Compiler Features
- [ ] Constant folding optimization
- [ ] Dead code elimination
- [ ] Loop unrolling
- [ ] Inline optimization
- [ ] SSA intermediate representation

### Multi-Target Support
- [ ] Windows (PE/COFF) backend
- [ ] Target abstraction layer
- [ ] Cross-compilation support

---

## v1.0.0 — Production Ready 🚀

**Target Date**: Q4 2028
**Focus**: Stability, Performance, Ecosystem

### Major Components

#### 1. Standard Library
- [ ] Math module (sqrt, pow, sin, cos, etc.)
- [ ] String utilities (parse, format, etc.)
- [ ] I/O module (file operations)
- [ ] Collections (Vector, HashMap)
- [ ] System module (environment, args, etc.)
- **Scope**: Substantial

#### 2. Multi-Platform Support
- [x] Linux x86-64 (v0.1.0)
- [ ] Windows x86-64 (PE/COFF)
- [ ] macOS x86-64 (Mach-O)
- [ ] macOS ARM64 (Apple Silicon)
- [ ] Linux ARM64

#### 3. Development Tools
- [ ] Language Server Protocol (LSP)
- [ ] Debugger support (GDB integration)
- [ ] Profiler for performance analysis
- [ ] Test framework
- [ ] Documentation generator

#### 4. Package Management
- [ ] Package manager ("vpm")
- [ ] Package registry
- [ ] Dependency resolution
- [ ] Semantic versioning

#### 5. Build System
- [ ] Project templates
- [ ] Build profiles (Debug/Release/Test)
- [ ] Build configuration files
- [ ] CI/CD integration examples

### Quality Metrics
- [ ] 95%+ test coverage
- [ ] Zero critical bugs
- [ ] Performance benchmarks established
- [ ] Production-grade documentation
- [ ] Stability guarantees

### Ecosystem
- [ ] Official website
- [ ] Community guidelines
- [ ] Third-party library ecosystem
- [ ] Learning resources

---

## Beyond v1.0.0 🔮

### Potential Future Directions

#### Language Features
- Async/await support
- Macros and metaprogramming
- First-class functions
- Lambda expressions
- Optional/Result types
- Context managers

#### Platform Support
- iOS development
- Android NDK support
- Embedded systems
- WebAssembly

#### Developer Experience
- IDE plugins (VSCode, IntelliJ, etc.)
- Visual debugging tools
- Performance profiler GUI
- Interactive REPL with history

#### Ecosystem Growth
- Web framework
- Game development framework
- Data processing libraries
- Scientific computing support

---

## Contributing to the Roadmap

### How to Influence Priorities

1. **Vote on GitHub Discussions**
   - Share your use cases
   - Explain why a feature matters to you
   - Help identify common patterns

2. **Request Features**
   - Open detailed feature requests
   - Include examples of proposed syntax
   - Explain implementation considerations

3. **Contribute**
   - Pick a roadmap item
   - Check [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines
   - Submit pull requests

4. **Sponsor Development**
   - Support core maintainers
   - Enable faster feature development

---

## Release Management

### Versioning Scheme
- **Major.Minor.Patch** (Semantic Versioning)
- Breaking changes require major version bump
- Feature additions use minor version bump
- Bug fixes use patch version bump

### Release Cycle
- **Minor releases**: Every 6 months
- **Patch releases**: As needed for bugs
- **Security releases**: Within 48 hours

### Beta/RC Process
- Beta releases for community feedback
- Release candidates before final release
- 2-week RC period minimum

---

## Revision History

| Date | Change |
|------|--------|
| 2026-09-03 | Initial roadmap created |

---

## Contact & Questions

- **Roadmap Discussion**: [GitHub Discussions](https://github.com/zyrndotio/Vel/discussions)
- **Bug Reports**: [GitHub Issues](https://github.com/zyrndotio/Vel/issues)
- **Direct Feedback**: Use GitHub issue templates

---

*Last Updated: 2026-09-03*
*Roadmap is subject to change based on community feedback and project priorities.*
