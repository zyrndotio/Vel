# Platform Support

Vel is split into two layers. The **portable frontend** includes the C++23 compiler executable, tokenizer, parser, syntax checker, token dump, and NASM assembly emission. The frontend is built and smoke-tested on Linux, macOS, and Windows.

The code generator emits x86-64 Linux ELF/NASM, macOS x86-64 Mach-O/NASM, and Windows x86-64 PE/COFF/NASM assembly. Linux x86-64 can assemble and link locally; macOS x86-64 can assemble and link with NASM and Apple `ld`; Windows x86-64 can assemble and link with NASM and MinGW-w64 or a compatible Win32 toolchain.

| Capability | Linux x86-64 | macOS | Windows |
|---|---:|---:|---:|
| Build the Vel compiler | Supported | Supported | Supported |
| `vel version` | Supported | Supported | Supported |
| `vel check` | Supported | Supported | Supported |
| `vel tokens` | Supported | Supported | Supported |
| `vel asm` | Supported | Supported | Supported |
| `vel build` | Supported | Supported on x86-64 | Supported with MinGW-w64 or compatible linker |
| Run generated Vel binaries | Supported | Supported on x86-64 | Supported on Windows x86-64 |

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

GitHub Actions also installs Inno Setup on the Windows runner, builds the installer, and uploads `VelSetup-windows-x64` as a CI artifact. The installer is per-user and does not require administrator privileges.

## Native backend roadmap

The target abstraction now covers Linux ELF, macOS Mach-O, and Windows PE/COFF. ARM64 targets remain the next backend milestone. The macOS path targets x86-64 and uses Darwin syscall numbers; the Windows path targets x86-64 and uses Win32 `WriteFile`, `GetStdHandle`, and `ExitProcess` imports.
