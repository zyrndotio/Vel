# Vel update design

Vel uses GitHub Releases as its public distribution channel. The updater queries the published `latest` release endpoint, selects an asset matching the host target, downloads it, and verifies the SHA-256 digest exposed by GitHub for that release asset before reporting success. It does not replace the running executable automatically.

This design follows GitHub’s release API model, where published releases expose `tag_name`, release assets, browser download URLs, and asset digests. GitHub documents the release endpoints at https://docs.github.com/en/rest/releases/releases. GitHub’s changelog explains that uploaded release assets expose immutable SHA-256 digests through the Releases UI, REST API, GraphQL API, and CLI: https://github.blog/changelog/2025-06-03-releases-now-expose-digests-for-release-assets/.

The next security milestone is signed release metadata and platform-native installation handoff. A checksum protects integrity against accidental corruption, but a future updater should also authenticate publisher intent with a signing key or a delegated signing framework before unattended replacement.

Current updater commands:

- `vel update` checks the latest release and reports whether the host is current.
- `vel update --download` downloads the matching archive, verifies its SHA-256 digest, and leaves installation to the user or platform package manager.

The updater currently supports the x86-64 Linux, macOS, and Windows portable archives published by the release workflow. It intentionally does not automatically install packages, overwrite a running binary, or make privileged changes.

## References

1. GitHub Docs, “REST API endpoints for releases”: https://docs.github.com/en/rest/releases/releases
2. GitHub Changelog, “Releases now expose digests for release assets”: https://github.blog/changelog/2025-06-03-releases-now-expose-digests-for-release-assets/
3. GitHub CLI Manual, “gh release download”: https://cli.github.com/manual/gh_release_download
