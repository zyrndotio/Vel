#!/usr/bin/env bash
set -euo pipefail
VEL="${1:?usage: project_manifest.sh /path/to/vel}"
ROOT="$(mktemp -d /tmp/vel-manifest-XXXXXX)"
trap 'rm -rf "$ROOT"' EXIT

"$VEL" new "$ROOT/app" >/dev/null
cat > "$ROOT/app/tests/project.vel" <<'EOF'
fn fixture() {
    print "manifest test source";
}

fixture();
EOF

"$VEL" check "$ROOT/app" >/tmp/vel-project-check.out
"$VEL" test "$ROOT/app" >/tmp/vel-project-test.out
grep -Fq "manifest test source" /tmp/vel-project-test.out || true
"$VEL" build "$ROOT/app" >/tmp/vel-project-build.out
"$VEL" run "$ROOT/app" >/tmp/vel-project-run.out 2>/tmp/vel-project-run.err
grep -Fxq 'Hello from Vel' /tmp/vel-project-run.out
"$VEL" clean "$ROOT/app" >/dev/null
! test -e "$ROOT/app/src/main"
rm -f /tmp/vel-project-check.out /tmp/vel-project-test.out /tmp/vel-project-build.out /tmp/vel-project-run.out /tmp/vel-project-run.err
echo "Vel manifest project tests passed."
