# Cross-Platform Desktop Application Quick Start

Vel can currently build native x86-64 programs for Linux, macOS, and Windows, and its frontend, syntax checker, and assembly emitter run on all three desktop operating systems. The compiler is therefore suitable for experimenting with native application logic and command-line tools today.

A complete first-party windowing and event API is **not yet part of the stable language**. The sample in [`templates/desktop-app`](../templates/desktop-app/) is an honest application foundation: it has a project manifest, a `src/main.vel` entry point, and a build/run workflow that works now. Window creation, graphics, menus, resources, and platform bundles remain roadmap work and should not be represented as already supported.

## 1. Install Vel

Download the matching compiler archive or Windows installer from the [latest Vel release](https://github.com/zyrndotio/Vel/releases/latest). Verify the SHA-256 checksum before placing the compiler on your `PATH`. Source-build instructions are in [BUILDING.md](BUILDING.md).

Verify the installation:

```bash
vel version
vel doctor
```

## 2. Create the application

Copy the template and enter its directory:

```bash
cp -R templates/desktop-app my-vel-app
cd my-vel-app
```

On PowerShell:

```powershell
Copy-Item -Recurse templates\desktop-app my-vel-app
Set-Location my-vel-app
```

## 3. Check, build, and run

The portable check works on every supported desktop operating system:

```bash
vel check src/main.vel
```

For a host-native executable, use:

```bash
vel build src/main.vel
```

On Linux and macOS, run the generated executable from the project directory:

```bash
./src/main
```

On Windows, run:

```powershell
.\src\main.exe
```

The generated binary is native to the host target. To produce a Windows or macOS assembly artifact from another host, use `vel asm src/main.vel windows-x86_64` or `vel asm src/main.vel macos-x86_64`; cross-linking and cross-compilation toolchain support are separate concerns.

## 4. Develop creatively while the GUI layer matures

The current stable surface is appropriate for native application cores, data-processing tools, generators, build utilities, and experiments that can use the standard platform process and file boundaries. Keep UI-specific code behind a small project boundary so the future Vel desktop API or an explicitly chosen external toolkit can replace the placeholder without rewriting application logic.

The next desktop milestones are a stable module/dependency system, structured diagnostics, a resource bundler, a window/event abstraction, and platform-native `.app`, `.deb`/`.rpm`, and Windows installer packaging. Track progress in [ECOSYSTEM_ROADMAP.md](ECOSYSTEM_ROADMAP.md).
