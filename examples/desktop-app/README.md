# Vel Desktop Application Example

This example is a complete **application-core project** using the current Vel project workflow. It is intentionally honest about the current platform boundary: Vel 0.3.1 can check, test, build, and run this native program, but it does not yet provide a first-party windowing, event, graphics, or resource-loading API.

## Run the example

From this directory:

```bash
vel check .
vel test .
vel build .
vel run .
```

The generated native executable is written beside `src/main.vel`. On Linux and macOS, run `./src/main`; on Windows, run `src\\main.exe`.

## Helper scripts

- `scripts/build.sh` is for Linux/macOS shells.
- `scripts/build.ps1` is for PowerShell on Windows.

Both scripts run checking, source tests, native compilation, and execution. They require a `vel` executable on `PATH` and the native assembler/linker tools described in the platform support guide.

## Application layers

The project is structured so future APIs can be added without changing the project boundary:

```text
src/main.vel       application entry point and core logic
tests/             portable Vel source fixtures
assets/            future application resources
scripts/           host build/run helpers
vel.toml           project manifest
```

When the standard library and desktop backend are implemented, filesystem, process, resource, window, and event code should be added behind this same layout rather than copied into compiler internals.
