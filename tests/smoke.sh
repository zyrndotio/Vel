#!/usr/bin/env bash
set -euo pipefail

VEL="${1:?usage: smoke.sh /path/to/vel}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

"$VEL" version
for example in "$ROOT"/examples/*.vel; do
    "$VEL" check "$example" >/dev/null
    "$VEL" tokens "$example" >/dev/null
    "$VEL" asm "$example" >/dev/null
done

echo "Vel frontend smoke tests passed."
