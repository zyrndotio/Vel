#!/usr/bin/env bash
set -euo pipefail

VEL="${1:?usage: continue_tests.sh /path/to/vel}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

"$VEL" check "$ROOT/tests/continue_nested.vel" >/dev/null
"$VEL" check "$ROOT/tests/functions_contracts.vel" >/dev/null
"$VEL" asm "$ROOT/tests/continue_nested.vel" >/tmp/vel-continue.asm
"$VEL" asm "$ROOT/tests/functions_contracts.vel" >/tmp/vel-functions.asm
"$VEL" asm "$ROOT/tests/functions_contracts.vel" macos-x86_64 >/tmp/vel-functions-macho.asm
grep -q 'section __TEXT,__text' /tmp/vel-functions-macho.asm
grep -q '0x2000004' /tmp/vel-functions-macho.asm
grep -q '0x2000001' /tmp/vel-functions-macho.asm

cat >/tmp/vel-bad-arity.vel <<'EOF'
fn add(a: int, b: int) -> int {
    return a + b;
}
print add(1);
EOF
if "$VEL" asm /tmp/vel-bad-arity.vel >/dev/null 2>/tmp/vel-bad-arity.err; then
    echo "Expected bad arity to fail" >&2
    exit 1
fi
grep -q "expects 2 argument(s), got 1" /tmp/vel-bad-arity.err

if [[ "$(uname -s)" == "Linux" && "$(uname -m)" == "x86_64" ]]; then
    "$VEL" build "$ROOT/tests/continue_nested.vel" >/dev/null
    [[ "$("$ROOT/tests/continue_nested")" == "7" ]]
    "$VEL" build "$ROOT/tests/functions_contracts.vel" >/dev/null
    [[ "$("$ROOT/tests/functions_contracts")" == $'427\n427' ]]
    rm -f "$ROOT/tests/continue_nested" "$ROOT/tests/functions_contracts"
fi

rm -f /tmp/vel-continue.asm /tmp/vel-functions.asm /tmp/vel-functions-macho.asm /tmp/vel-bad-arity.vel /tmp/vel-bad-arity.err
echo "Vel continue, function, and Mach-O assembly tests passed."
