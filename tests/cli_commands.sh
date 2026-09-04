#!/usr/bin/env bash
set -euo pipefail
VEL="${1:?usage: cli_commands.sh /path/to/vel}"
ROOT="$(mktemp -d /tmp/vel-project-XXXXXX)"
trap 'rm -rf "$ROOT"' EXIT

"$VEL" doctor >/tmp/vel-doctor.out
"$VEL" new "$ROOT/app" >/dev/null
[[ -f "$ROOT/app/vel.toml" ]]
[[ -f "$ROOT/app/src/main.vel" ]]
"$VEL" check "$ROOT/app/src/main.vel" >/dev/null
"$VEL" run "$ROOT/app/src/main.vel" >/tmp/vel-run.out 2>/tmp/vel-run.err
grep -Fxq 'Hello from Vel' /tmp/vel-run.out
"$VEL" clean "$ROOT/app/src/main.vel" >/dev/null
rm -f /tmp/vel-doctor.out /tmp/vel-run.out /tmp/vel-run.err
echo "Vel CLI project command tests passed."
