# Platform Support

Vel is split into two layers. The **portable frontend** includes the C++23 compiler executable, tokenizer, parser, syntax checker, token dump, and NASM assembly emission. The frontend is built and smoke-tested on Linux, macOS, and Windows.

The **native backend** currently emits x86-64 Linux NASM and links with Linux GNU binutils. Consequently, `vel build` and the shorthand `vel program.vel` require Linux x86-64, NASM, and `ld`. On macOS and Windows, use `vel check`, `vel tokens`, or `vel asm` until PE/COFF and Mach-O backends are implemented.

| Capability | Linux x86-64 | macOS | Windows |
|---|---:|---:|---:|
| Build the Vel compiler | Supported | Supported | Supported |
| `vel version` | Supported | Supported | Supported |
| `vel check` | Supported | Supported | Supported |
| `vel tokens` | Supported | Supported | Supported |
| `vel asm` | Supported | Supported | Supported |
| `vel build` | Supported | Planned | Planned |
| Run generated Vel binaries | Supported | Planned | Planned |

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

The next backend milestone is to introduce a target abstraction in code generation, then add platform-specific startup, output, object-file, and linker implementations for macOS Mach-O and Windows PE/COFF. Until those backends land, the portable frontend is the supported way to use Vel on non-Linux hosts.
