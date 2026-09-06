# Native Interoperability

Vel keeps the compiler core in C++23, emits target assembly for the supported x86-64 backends, and provides a small C ABI host shim for platform services. This lets projects use the right language at the right boundary without splitting the compiler into multiple runtimes.

## Current capabilities

```bash
vel asm projects/console/hello.vel > hello.asm
vel build projects/console/hello.vel
```

`vel asm` emits NASM-compatible assembly for the selected target. The compiler uses NASM and the platform linker to produce a native executable. The generated assembly is an intermediate artifact and should be reviewed as target-specific output rather than treated as a stable source-level ABI.

The packaged `runtime/vel_std.h` header and `vel_std` library provide the current C ABI foundation for environment, filesystem, and process operations. Vel source cannot yet declare `extern` functions or import C modules; module and FFI syntax will be designed after ownership and typed error semantics are stable.

## C and assembly project layout

Future native application projects should keep platform-specific code in one project tree:

```text
projects/my-app/
  src/                 Vel source
  native/              optional C or assembly adapters
  assets/              application resources
  tests/               Vel and native integration tests
  vel.toml             project manifest
```

Native adapters must have an explicit ownership contract, target guards, and tests. They must not bypass bounds checks or expose raw platform handles directly to Vel code.

## Security and performance rules

The compiler rejects source files larger than 64 MiB to bound frontend memory use. Release compiler builds enable stack-protection and fortified libc checks; Linux release binaries also use full RELRO and immediate symbol binding. Maintainers can configure `-DVEL_ENABLE_SANITIZERS=ON` for AddressSanitizer and UndefinedBehaviorSanitizer validation.

The frontend arena is intentionally bulk-reclaimed after each compiler invocation. Current sanitizer profiles disable LeakSanitizer for this known lifetime model while retaining AddressSanitizer and UndefinedBehaviorSanitizer checks. A future arena ownership pass should add destructor-aware cleanup for non-trivial AST containers.

The provisional process shim accepts a shell command string and must not receive untrusted input. A future process API must use an argument vector and normalized exit status before it is exposed as a stable Vel module.
