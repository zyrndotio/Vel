# Native adapters

This directory is reserved for optional C or assembly adapters used by the desktop application. Vel v0.3.2 does not yet expose `extern` declarations or module imports, so files placed here are not linked automatically.

For the current release, generate and inspect target assembly with:

```bash
vel asm ../src/main.vel > main.asm
```

When FFI syntax and typed ownership rules are implemented, native adapters will be linked through the project manifest and covered by platform-specific integration tests. Do not pass untrusted input to the provisional shell-based process host function.
