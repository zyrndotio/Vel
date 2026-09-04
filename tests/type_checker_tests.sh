#!/usr/bin/env bash
set -euo pipefail

VEL="${1:?usage: type_checker_tests.sh /path/to/vel}"

expect_failure() {
    local name="$1"
    local source="$2"
    local expected="$3"
    printf '%s\n' "$source" >/tmp/vel-type-check-${name}.vel
    if "$VEL" check /tmp/vel-type-check-${name}.vel >/dev/null 2>/tmp/vel-type-check-${name}.err; then
        echo "Expected $name to fail" >&2
        exit 1
    fi
    grep -Fq "$expected" /tmp/vel-type-check-${name}.err
    rm -f /tmp/vel-type-check-${name}.vel /tmp/vel-type-check-${name}.err
}

expect_failure wrong_arg_type 'fn add(a: int) -> int { return a; } print add(true);' "argument 1 of 'add' expects int, got bool"
expect_failure missing_return 'fn answer() -> int { print 42; }' "must return a value of type int"
expect_failure void_expression 'fn announce(value: int) { print value; } let x = announce(1);' "void function 'announce' cannot be used as an expression"
expect_failure break_outside 'break;' 'break used outside of a loop'
expect_failure continue_outside 'continue;' 'continue used outside of a loop'

echo "Vel type-checker negative tests passed."
