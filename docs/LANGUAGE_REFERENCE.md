# Language Reference

Complete specification of the Vel programming language.

---

## Table of Contents

1. [Syntax Overview](#syntax-overview)
2. [Lexical Elements](#lexical-elements)
3. [Types](#types)
4. [Variables](#variables)
5. [Operators](#operators)
6. [Statements](#statements)
7. [Functions](#functions)
8. [Expressions](#expressions)
9. [Comments](#comments)

---

## Syntax Overview

Vel is a C-style language with familiar syntax:

```vel
// Single statement
print "Hello";

// Block with multiple statements
{
    let x = 42;
    print x;
}

// Function definition
fn add(a: int, b: int) -> int {
    return a + b;
}
```

---

## Lexical Elements

### Keywords

Reserved words that cannot be used as identifiers:

```
let   mut   fn    return   if    elif   else   while   loop   break   continue   print
```

### Identifiers

Names for variables, functions, and types:

- Start with letter (a-z, A-Z) or underscore (_)
- Followed by letters, digits (0-9), or underscores
- Case-sensitive: `x` and `X` are different

Valid: `name`, `_private`, `my_var`, `value2`
Invalid: `2value`, `my-var`, `my var`

### Literals

#### Integer Literals
```vel
let decimal = 42;
let large = 1000000;
let negative = -7;
```

#### Float Literals
```vel
let pi = 3.14159;
let scientific = 2.5e-3;  // 0.0025
let negative = -0.5;
```

#### String Literals
```vel
let greeting = "Hello, World!";
let empty = "";
```

Characters in double quotes, escape sequences not yet supported.

#### Boolean Literals
```vel
let yes = true;
let no = false;
```

---

## Types

### Primitive Types

#### `int` — Integer Type
- 64-bit signed integer
- Range: -2^63 to 2^63 - 1
- Literals: `42`, `-7`, `0`

#### `float` — Floating Point Type
- 64-bit IEEE 754 double precision
- Literals: `3.14`, `-0.5`, `2.5e-3`

#### `str` — String Type
- UTF-8 encoded text
- Immutable sequences of characters
- Literals: `"hello"`, `""`

#### `bool` — Boolean Type
- Two values: `true` or `false`

### Type Annotations

Variables can have explicit type annotations:

```vel
let x: int = 42;           // Explicitly typed
let y = 42;                // Type inferred (v0.2.0+)
let name: str = "Vel";     // String type
let active: bool = true;   // Boolean type
```

Syntax: `name: type = value`

---

## Variables

### Immutable Variables

Use `let` for immutable binding:

```vel
let x = 42;
let name = "Vel";
let ratio: float = 3.14;

// Cannot reassign
x = 100;  // ERROR: immutable
```

### Mutable Variables

Use `mut` for mutable variables:

```vel
mut count = 0;
mut value: int = 42;

// Can reassign
count = 1;
count = count + 1;  // OK
```

### Variable Scope

Variables are scoped to their block:

```vel
{
    let x = 42;
    print x;  // OK
}
print x;      // ERROR: x not in scope
```

---

## Operators

### Arithmetic Operators

| Operator | Name | Example | Result |
|----------|------|---------|--------|
| `+` | Addition | `10 + 5` | `15` |
| `-` | Subtraction | `10 - 5` | `5` |
| `*` | Multiplication | `10 * 5` | `50` |
| `/` | Division | `10 / 5` | `2` |
| `%` | Modulo | `10 % 3` | `1` |

```vel
let sum = 10 + 5;        // 15
let diff = 10 - 5;       // 5
let product = 10 * 5;    // 50
let quotient = 10 / 5;   // 2
let remainder = 10 % 3;  // 1
```

### Comparison Operators

| Operator | Meaning | Example | Result |
|----------|---------|---------|--------|
| `==` | Equal | `5 == 5` | `true` |
| `!=` | Not equal | `5 != 3` | `true` |
| `<` | Less than | `3 < 5` | `true` |
| `>` | Greater than | `5 > 3` | `true` |
| `<=` | Less or equal | `5 <= 5` | `true` |
| `>=` | Greater or equal | `5 >= 5` | `true` |

```vel
if (x == 5) { print "equal"; }
if (x != 5) { print "not equal"; }
if (x < 10) { print "less"; }
```

### Logical Operators

| Operator | Name | Example | Meaning |
|----------|------|---------|---------|
| `&&` | AND | `a && b` | Both true |
| `\|\|` | OR | `a \|\| b` | Either true |
| `!` | NOT | `!a` | Opposite |

```vel
if (x > 0 && x < 10) { print "in range"; }
if (x < 0 || x > 100) { print "out of range"; }
if (!active) { print "inactive"; }
```

### Assignment Operator

```vel
let x = 42;      // Initial assignment
mut y = 0;
y = 100;         // Reassignment (mut only)
y = y + 1;       // Compound: y = 101
```

### Precedence

From highest to lowest:

1. `()` - Parentheses
2. Unary: `!`, `-`
3. `*`, `/`, `%`
4. `+`, `-`
5. `<`, `>`, `<=`, `>=`
6. `==`, `!=`
7. `&&`
8. `||`
9. `=` - Assignment

---

## Statements

### Variable Declaration

```vel
let x = 42;                    // Immutable
mut y = 0;                     // Mutable
let z: int = 100;             // With type
```

### Expression Statement

```vel
let result = 10 + 5;   // Expression becomes statement
print result;          // Function call statement
```

### Block Statement

```vel
{
    let x = 42;
    print x;
}
```

### Print Statement

Built-in print function:

```vel
print 42;              // Integer
print "hello";         // String
print true;            // Boolean
```

Always outputs followed by newline.

### Assignment Statement

```vel
mut x = 0;
x = 42;                // Reassign
x = x + 1;             // Compound
```

### If Statement

```vel
if (condition) {
    // ...
}

if (condition) {
    // ...
} else {
    // ...
}

if (condition1) {
    // ...
} elif (condition2) {
    // ...
} else {
    // ...
}
```

Conditions must be boolean expressions.

### While Loop

```vel
while (condition) {
    // Loop body
    // Repeats while condition is true
}
```

### Infinite Loop

```vel
loop {
    // Always executes
    if (should_break) {
        break;     // Exit loop
    }
}
```

### Break Statement

Exits innermost loop:

```vel
mut i = 0;
while (true) {
    if (i == 5) {
        break;  // Exit loop
    }
    i = i + 1;
}
```

### Continue Statement

Skips to next iteration:

```vel
mut i = 0;
while (i < 10) {
    i = i + 1;
    if (i == 5) {
        continue;  // Skip to next iteration
    }
    print i;
}
```

### Return Statement

Returns from function:

```vel
fn get_answer() -> int {
    return 42;
}

fn greet() {
    print "Hello";
    return;      // Early exit
}
```

---

## Functions

### Function Definition

```vel
fn function_name(param1: type1, param2: type2) -> return_type {
    // Function body
    return value;
}
```

### Parameters

```vel
fn add(a: int, b: int) -> int {
    return a + b;
}

fn greet(name: str) {
    print "Hello, ";
    print name;
}
```

Types are required for all parameters. Calls must provide exactly the declared number of arguments, and the type checker validates scalar argument types before code generation. Array and named-struct parameter syntax is parsed and represented in the type system; aggregate runtime calling conventions remain under development.

### Return Type

```vel
fn get_number() -> int {
    return 42;
}

fn greet() {
    print "Hello";
    // Implicit return (void)
}
```

Use `->` to specify a return type. A non-void function must contain a value-returning statement, while a void function may only use `return;` or an implicit return.

### Return Values

```vel
fn add(a: int, b: int) -> int {
    return a + b;  // Explicit return
}

let sum = add(3, 4);  // 7
```

### Function Calls

```vel
fn greet(name: str) {
    print "Hello, ";
    print name;
}

greet("World");  // Call with argument
```

Arguments must match the declared arity and declared scalar types. Integer, boolean, and string values are currently represented through the backend’s register conventions. Aggregate arguments are accepted by the frontend type model but are not yet emitted by native backends.

### Arrays, Structs, and String Concatenation

The frontend accepts homogeneous array literals and recursive array annotations:

```vel
let numbers: [int] = [1, 2, 3];
let first: int = numbers[0];
```

Named structs use field declarations and struct literals:

```vel
struct Point {
    x: int,
    y: int,
}

let origin: Point = Point { x: 0, y: 0 };
let x = origin.x;
```

The `+` operator supports concatenating two strings:

```vel
let message: str = "Hello, " + "Vel";
```

The parser and type checker validate these constructs, including homogeneous array elements and struct field names/types. Native array and struct storage/layout code generation is the next implementation stage.

### Variable Scope in Functions

```vel
fn example() {
    let x = 42;    // Local variable
}

print x;           // ERROR: x not in scope
```

---

## Expressions

### Arithmetic Expressions

```vel
let result = 10 + 5 * 2;           // 20 (precedence)
let result = (10 + 5) * 2;         // 30 (parentheses)
```

### Comparison Expressions

```vel
let is_equal = (5 == 5);           // true
let is_less = (3 < 5);             // true
```

### Logical Expressions

```vel
let both = (true && true);         // true
let either = (true || false);      // true
let not = !false;                  // true
```

### Function Call Expressions

```vel
let result = add(3, 4);            // Evaluates to 7
let doubled = double(result);      // Nested calls
```

---

## Comments

### Line Comments

```vel
// This is a comment
let x = 42;  // Inline comment
```

### Block Comments

```vel
/* This is a
   multi-line comment */

let x = /* inline comment */ 42;
```

---

## Grammar Summary

```ebnf
program = (statement)*

statement = variable_decl
          | expression_stmt
          | print_stmt
          | if_stmt
          | while_stmt
          | loop_stmt
          | break_stmt
          | continue_stmt
          | return_stmt
          | block
          | function_def

expression = logical_or

logical_or = logical_and ("||" logical_and)*

logical_and = equality ("&&" equality)*

equality = comparison (("==" | "!=") comparison)*

comparison = term (("<" | ">" | "<=" | ">=") term)*

term = factor (("+" | "-") factor)*

factor = unary (("*" | "/" | "%") unary)*

unary = ("!" | "-") unary | primary

primary = number | string | boolean | identifier | "(" expression ")"
```

---

## Standard Functions

### Print Function

```vel
print value;   // Outputs value followed by newline
```

Accepts: `int`, `float`, `str`, `bool`

---

## Best Practices

✅ **DO:**
- Use meaningful variable names
- Type annotate function parameters
- Add comments for complex logic
- Use immutable by default (`let`)
- Keep functions focused

❌ **DON'T:**
- Use cryptic single-letter variables
- Omit type annotations
- Nest too deeply
- Create very long functions

---

## Common Patterns

### Counter Pattern
```vel
mut count = 0;
while (count < 10) {
    print count;
    count = count + 1;
}
```

### Conditional Pattern
```vel
if (value > 100) {
    print "high";
} elif (value > 50) {
    print "medium";
} else {
    print "low";
}
```

### Function Pattern
```vel
fn process(value: int) -> int {
    return value * 2;
}

let result = process(21);
print result;  // 42
```

---

## What's Not Yet Supported

- ❌ String concatenation (v0.2.0+)
- ❌ Arrays (v0.2.0+)
- ❌ Structs (v0.3.0+)
- ❌ Generics (v0.4.0+)
- ❌ Module system (v0.3.0+)

---

<div align="center">

**For more**: [CLI_REFERENCE.md](CLI_REFERENCE.md) · [TESTING.md](TESTING.md)

</div>

---

*Last Updated: 2026-09-03*
