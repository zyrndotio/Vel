# Vel Ecosystem Roadmap

Vel's primary product goal is a **creative, cross-platform systems language for desktop applications**. That requires more than a compiler: developers need dependable installation, project conventions, editor tooling, native application APIs, packaging, and a welcoming contribution path.

## Phase 1 — Discoverable and installable

- Keep signed, reproducible release archives for Linux, macOS, and Windows.
- Publish SHA-256 checksums and provenance metadata for every release.
- Maintain the Windows installer and portable archive.
- Add package-manager definitions where the project has maintainers able to review them: Homebrew, winget, and Linux distributions.
- Keep `.vel` GitHub Linguist metadata and a first-party VS Code extension in the repository.

## Phase 2 — Productive projects

- Stabilize `vel.toml` as the project manifest.
- Add dependency resolution, lockfiles, modules, and workspace support.
- Implement `vel test`, `vel fmt`, and structured diagnostics.
- Add VS Code tasks and a Language Server Protocol implementation.
- Provide templates for a command-line tool, a GUI application, and a desktop application with assets.

## Phase 3 — Desktop application foundation

- Define a small, portable application API for windows, events, drawing surfaces, files, processes, and concurrency.
- Keep platform-specific implementations behind stable Vel-facing interfaces.
- Decide deliberately whether the first-party GUI layer should use native APIs, a portable C library, or an optional external toolkit. The compiler should not require a managed runtime merely to build a desktop application.
- Add resource bundling and platform packaging for `.app`, `.deb`/`.rpm`, and Windows installers.
- Ship at least one real sample application built and packaged on all supported platforms.

## Phase 4 — Professional tooling

- Add debugger integration through the Debug Adapter Protocol.
- Add profiling and crash-reporting hooks that preserve user control and privacy.
- Support reproducible builds, signing guidance, upgrade/rollback behavior, and stable ABI documentation.
- Expand editor integrations using the shared grammar and language server.

## Acceptance criteria for a 1.0 ecosystem

A new developer should be able to install Vel without building C++, create a project, open it in a supported editor, receive useful diagnostics, run tests, build a native desktop application, and produce a platform-appropriate distributable from a documented command sequence. Each step must work on Linux, macOS, and Windows or clearly state its platform limitation.
