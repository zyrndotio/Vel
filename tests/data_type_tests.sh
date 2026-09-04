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

if [[ "$(uname -s)" == "Linux" && "$(uname -m)" == "x86_64" ]]; then
    "$VEL" build "$ROOT/tests/aggregates_complex.vel" >/dev/null
    output="$($ROOT/tests/aggregates_complex)"
    expected=$'3\nAB\nAB'
    [[ "$output" == "$expected" ]]
    rm -f "$ROOT/tests/aggregates_complex"
fi

echo "Vel aggregate and string type-checker tests passed."
