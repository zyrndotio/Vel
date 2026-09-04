# Platform Support

Vel is split into two layers. The **portable frontend** includes the C++23 compiler executable, tokenizer, parser, syntax checker, token dump, and NASM assembly emission. The frontend is built and smoke-tested on Linux, macOS, and Windows.

The code generator now emits x86-64 Linux ELF/NASM and macOS x86-64 Mach-O/NASM assembly. Linux x86-64 can assemble and link locally; macOS x86-64 can assemble and link with NASM and Apple `ld`. Windows remains frontend-only until a PE/COFF backend is added.

| Capability | Linux x86-64 | macOS | Windows |
|---|---:|---:|---:|
| Build the Vel compiler | Supported | Supported | Supported |
| `vel version` | Supported | Supported | Supported |
| `vel check` | Supported | Supported | Supported |
| `vel tokens` | Supported | Supported | Supported |
| `vel asm` | Supported | Supported | Supported |
| `vel build` | Supported | Supported on x86-64 | Planned |
| Run generated Vel binaries | Supported | Supported on x86-64 | Planned |

## Build from source

Use the same CMake commands on all three desktop operating systems:

```bash
cmake -S . -B build
cmake --build build --config Release
```

On Unix-like systems, run the frontend smoke test with:

```bash
ctest --test-dir build --output-on-failure
```

On Windows, the executable is located at `build\\Release\\vel.exe`; on single-configuration Unix generators it is usually `build/vel`.

## Windows installer

The repository includes `vel_installer.iss` for Inno Setup. It creates a per-user `VelSetup-x64.exe` installer, places the compiler under the user profile, and updates the current user’s PATH without requiring administrator privileges.

```powershell
cmake -S . -B build
cmake --build build --config Release
ISCC.exe vel_installer.iss
```

## Native backend roadmap

The target abstraction and macOS Mach-O emission path are now in place. The next backend milestone is Windows PE/COFF support, followed by ARM64 targets. The macOS path currently targets x86-64 and uses Darwin syscall numbers for the built-in output routines.
