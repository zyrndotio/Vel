#!/usr/bin/env bash
set -euo pipefail
VEL="${1:?usage: data_type_tests.sh /path/to/vel}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

"$VEL" check "$ROOT/tests/data_types.vel" >/dev/null
"$VEL" check "$ROOT/tests/aggregates_complex.vel" >/dev/null

expect_failure() {
    local name="$1"
    local source="$2"
    local expected="$3"
    local file="/tmp/vel-data-${name}.vel"
    local err="/tmp/vel-data-${name}.err"
    printf '%s\n' "$source" >"$file"
    if "$VEL" check "$file" >/dev/null 2>"$err"; then
        echo "Expected $name to fail" >&2
        exit 1
    fi
    grep -Fq "$expected" "$err"
    rm -f "$file" "$err"
}

expect_failure mixed_array 'let values: [int] = [1, true];' 'array elements must have the same type'
expect_failure bad_index 'let values: [int] = [1, 2]; let x: int = values[true];' 'array index must be int'
expect_failure missing_field 'struct Point { x: int, y: int } let p: Point = Point { x: 1 };' "missing a field"
expect_failure unknown_field 'struct Point { x: int } let p: Point = Point { x: 1, y: 2 };' "has no field 'y'"
expect_failure bad_concat 'let value: str = "hello" + 1;' 'operator + requires matching numeric types or two strings'
expect_failure immutable_append 'let values: [int] = [1, 2]; append(values, 3);' "cannot append to immutable array"
expect_failure immutable_indexed_write 'let values: [int] = [1, 2]; values[0] = 3;' "cannot mutate immutable array"

if [[ "$(uname -s)" == "Linux" && "$(uname -m)" == "x86_64" ]]; then
    "$VEL" build "$ROOT/tests/aggregates_complex.vel" >/dev/null
    output="$($ROOT/tests/aggregates_complex)"
    expected=$'3\nAB\nAB'
    [[ "$output" == "$expected" ]]
    rm -f "$ROOT/tests/aggregates_complex"

    mutation_file="$(mktemp /tmp/vel-mutation-XXXXXX.vel)"
    mutation_output="${mutation_file%.vel}"
    printf 'mut values: [int] = [1, 2, 3];\nvalues[1] = 20;\nappend(values, 4);\nprint values[1];\nprint values[2];\nprint values[3];\n' >"$mutation_file"
    "$VEL" build "$mutation_file" >/dev/null
    mutation_result="$($mutation_output)"
    [[ "$mutation_result" == $'20\n3\n4' ]]
    rm -f "$mutation_file" "$mutation_output"

    # Exercise the runtime allocator beyond the former 64 KiB concat buffer.
    long_file="$(mktemp /tmp/vel-long-XXXXXX.vel)"
    long_output="${long_file%.vel}"
    long_text="$(head -c 70000 /dev/zero | tr '\0' 'A')"
    printf 'let long: str = "%s" + "B";\nprint long;\n' "$long_text" >"$long_file"
    "$VEL" build "$long_file" >/dev/null
    "$long_output" >"${long_output}.out"
    [[ "$(wc -c <"${long_output}.out")" -eq 70002 ]]
    rm -f "$long_file" "$long_output" "${long_output}.out"

    bounds_file="$(mktemp /tmp/vel-bounds-XXXXXX.vel)"
    bounds_output="${bounds_file%.vel}"
    printf 'let values: [int] = [1, 2];\nprint values[2];\n' >"$bounds_file"
    "$VEL" build "$bounds_file" >/dev/null
    if "$bounds_output" >/dev/null 2>&1; then
        echo "Expected upper-bound access to fail" >&2
        exit 1
    fi
    rm -f "$bounds_file" "$bounds_output"

    negative_file="$(mktemp /tmp/vel-negative-XXXXXX.vel)"
    negative_output="${negative_file%.vel}"
    printf 'let values: [int] = [1, 2];\nprint values[-1];\n' >"$negative_file"
    "$VEL" build "$negative_file" >/dev/null
    if "$negative_output" >/dev/null 2>&1; then
        echo "Expected negative index access to fail" >&2
        exit 1
    fi
    rm -f "$negative_file" "$negative_output"
fi

echo "Vel aggregate and string type-checker tests passed."
