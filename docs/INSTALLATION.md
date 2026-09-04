# Installing Vel

Vel is distributed as a native compiler for **Linux x86-64**, **macOS x86-64**, and **Windows x86-64**. The official release page is the source of truth for versioned downloads and checksums.

## Download a release

Open the [latest Vel release](https://github.com/zyrndotio/Vel/releases/latest) and choose the archive for your operating system:

| Platform | Package | Contents |
|---|---|---|
| Linux x86-64 | `.tar.gz` | `vel` compiler, documentation, examples, and assets |
| macOS x86-64 | `.tar.gz` | `vel` compiler, documentation, examples, and assets |
| Windows x86-64 | `.zip` | Portable compiler bundle |
| Windows x86-64 | `VelSetup-x64.exe` | Inno Setup installer |

After downloading, verify the SHA-256 digest shown by the release before installing. The `vel update --download` command also verifies the digest of a downloaded archive, but it deliberately does not replace the running compiler or make privileged changes.

## Linux and macOS

Extract the archive, place the `vel` executable in a directory on your `PATH`, and verify the installation:

```bash
./vel version
./vel doctor
./vel check examples/hello.vel
```

For a system-wide installation, use your operating system's normal package-management policy or copy the executable to a managed directory such as `/usr/local/bin` after reviewing the release and its checksum.

## Windows

Run `VelSetup-x64.exe` for an installed setup, or extract the portable `.zip` and invoke `vel.exe` directly. Verify from PowerShell:

```powershell
vel.exe version
vel.exe doctor
vel.exe check examples\hello.vel
```

## Build from source

Building from source requires a C++23 compiler and CMake 3.22 or newer. Native linking additionally requires NASM and the platform linker. See [BUILDING.md](BUILDING.md) for platform-specific details.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Editor support

Vel source files use the `.vel` extension. The **Vel Language Support** extension is delivered separately as `vel-language-vscode-0.3.0.zip`, so the compiler repository remains focused on the language and native toolchain. Extract it and package it as a VSIX for local distribution:

```bash
unzip vel-language-vscode-0.3.0.zip -d vel-language-vscode
cd vel-language-vscode
npx @vscode/vsce package
code --install-extension vel-language-0.3.0.vsix
```

The extension provides syntax highlighting, comments, bracket matching, indentation, and starter snippets. It does not yet provide a language server, debugger, formatter, or one-click build task; those are tracked in the ecosystem roadmap.
