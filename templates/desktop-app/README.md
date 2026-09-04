# Vel Desktop App Template

This is a minimal cross-platform Vel application foundation. It is intentionally limited to the stable Vel 0.3.1 language and compiler surface, so it can be checked and built on supported desktop targets today.

## Commands

```bash
vel check .
vel test .
vel build .
vel run .
```

The manifest resolves `src/main.vel` as the entry point and `tests/` as the source-test directory. Run `./src/main` on Linux/macOS or `src\\main.exe` on Windows after a successful native build. Windowing, event dispatch, resources, and platform application bundles are planned rather than included in this starter.
