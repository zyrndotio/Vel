# Vel Standard Library Design

This document defines the first standard-library boundary planned for v0.3.2. The low-level cross-platform host shim is now implemented and tested, but these modules are not yet exposed as Vel `use` imports because module syntax and typed result values are still under development. APIs become stable only after they have target implementations and regression tests on Linux, macOS, and Windows.

## Design principles

The library must be small, explicit, synchronous by default, and independent of a managed runtime. Host-specific code belongs behind target-neutral Vel APIs. Fallible operations must not silently return fabricated values.

## Initial modules

| Module | Initial responsibility | Status |
|---|---|---|
| `std::args` | Program arguments and executable path | Design |
| `std::env` | Environment lookup and current directory | Host shim foundation |
| `std::path` | Joining, splitting, and normalizing paths | Design |
| `std::fs` | File existence, read, write, and directory listing | Host shim foundation |
| `std::process` | Spawn a process and observe its exit status | Host shim foundation |
| `std::time` | Wall-clock time and monotonic duration | Design |
| `std::io` | Explicit byte/text streams and standard handles | Design |

## Proposed semantics

The first API should make failure visible. Until Vel has a stable `Result` type and module imports, these signatures are illustrative:

```vel
use std::env;
use std::fs;
use std::path;

fn main() {
    let root = env.current_dir();
    let file = path.join(root, "settings.vel");
    if fs.exists(file) {
        print fs.read_text(file);
    }
}
```

The implementation must not hide permission failures, missing files, invalid paths, encoding errors, or process-launch failures. The final surface should use a typed result/error model rather than process termination for normal application errors.

## Host boundary

The compiler should expose a small internal host ABI with target-specific implementations:

```text
Vel standard library
    ↓ stable library ABI
Vel host shim
    ↓ target adapter
Linux syscalls / libc     macOS system APIs     Windows APIs
```

The host shim must define path encoding, file handle ownership, process exit status, time units, and cleanup behavior. It must not leak Linux syscall numbers or Windows handles into source-level APIs.

The current `vel_process_run` host-shim function is explicitly provisional: it accepts a command string through the host shell and must not receive untrusted input. A stable `std::process` module must replace this with an argument-vector API that avoids shell injection and normalizes exit status across targets.

## Implementation order

1. Add module/import syntax and a symbol-resolution model.
2. Add a typed result/error representation.
3. Implement `std::args` and `std::env` because they have small platform surfaces.
4. Implement `std::path` as a pure library layer with deterministic tests.
5. Implement `std::fs` and `std::io` with explicit resource cleanup.
6. Implement `std::process` and `std::time`.
7. Add desktop resource paths and window/event interfaces on top of the same host boundary.

## Packaging consequence

The v0.3.2 packages now include the public `vel_std.h` header and the static `vel_std` host shim library. The higher-level Vel modules remain unavailable until imports and typed errors are implemented. The Windows Inno Setup script, CPack metadata, and macOS/Linux package manifests must be updated together when those source-level modules become available.
