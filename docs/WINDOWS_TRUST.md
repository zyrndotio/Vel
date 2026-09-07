# Windows Installer Trust

Vel's installer now identifies **Zyrndotio** as the publisher and links to the official GitHub repository, support page, and release page. This improves transparency and gives Windows users a consistent product identity.

Publisher metadata alone cannot remove Microsoft Defender SmartScreen warnings. SmartScreen trust requires a real Authenticode code-signing certificate, secure private-key handling, and accumulated reputation. A self-signed certificate usually produces a warning and should not be presented as trusted distribution.

## Maintainer checklist

1. Obtain an Authenticode certificate from a recognized certificate authority or an approved organizational signing provider. See the [Windows signing guide](WINDOWS_SIGNING.md) for local testing and production options.
2. Store the certificate and password outside the repository, using protected GitHub Actions secrets or a hardware-backed signing service.
3. Sign `vel.exe` and `VelSetup-x64.exe` after building them. The release workflow signs these files when the protected certificate secrets are configured.
4. Timestamp signatures using a trusted RFC 3161 timestamp service so signatures remain verifiable after certificate expiration.
5. Publish SHA-256 checksums and verify that the signed installer checksum matches the release asset.
6. Submit the signed installer through Microsoft's publisher/reputation processes when SmartScreen continues to warn on a new product.

The repository does **not** contain a certificate, private key, or signing password. The release workflow should only add a signing step after the maintainer supplies those credentials as protected secrets. Adding a fake publisher name or disabling security warnings would not make the installer trustworthy.
