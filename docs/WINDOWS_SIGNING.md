# Windows Code Signing for Vel

This guide covers two different goals:

1. **Local testing:** create a self-signed certificate and make your own Windows account trust it.
2. **Public distribution:** obtain a certificate or managed signing service that identifies **Zyrndotio** to users.

A self-signed certificate is useful for testing the signing pipeline, but it does not remove SmartScreen warnings for public downloads. Microsoft documents that self-signed certificates behave like unsigned files for SmartScreen reputation. A public certificate can display the verified publisher name, but a newly released file can still produce an initial SmartScreen warning while reputation accumulates.

## Local self-signed testing

Run PowerShell as the account that will test the installer. The certificate is deliberately limited to code signing and expires after one year:

```powershell
$cert = New-SelfSignedCertificate `
  -Type CodeSigningCert `
  -Subject 'CN=Zyrndotio Local Test' `
  -FriendlyName 'Zyrndotio Vel Local Test Signing' `
  -CertStoreLocation 'Cert:\CurrentUser\My' `
  -NotAfter (Get-Date).AddYears(1)

$password = Read-Host 'PFX password' -AsSecureString
Export-PfxCertificate `
  -Cert $cert `
  -FilePath "$HOME\vel-local-test.pfx" `
  -Password $password
Export-Certificate `
  -Cert $cert `
  -FilePath "$HOME\vel-local-test.cer"
```

Trust the public certificate only on the development machine:

```powershell
Import-Certificate `
  -FilePath "$HOME\vel-local-test.cer" `
  -CertStoreLocation 'Cert:\CurrentUser\TrustedPublisher'
```

Locate `signtool.exe` from the Windows SDK, then sign and verify a locally built installer. Do not commit the PFX file:

```powershell
$signTool = 'C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe'
$pfx = "$HOME\vel-local-test.pfx"
$pfxPassword = Read-Host 'PFX password' -AsSecureString
$plainPassword = [Runtime.InteropServices.Marshal]::PtrToStringBSTR(
  [Runtime.InteropServices.Marshal]::SecureStringToBSTR($pfxPassword))

& $signTool sign /fd SHA256 /td SHA256 `
  /tr http://timestamp.digicert.com `
  /f $pfx /p $plainPassword `
  /d 'Vel Programming Language Local Test' `
  build\Release\vel.exe

& $signTool sign /fd SHA256 /td SHA256 `
  /tr http://timestamp.digicert.com `
  /f $pfx /p $plainPassword `
  /d 'Vel Installer Local Test' `
  installer_output\VelSetup-x64.exe

& $signTool verify /pa /all build\Release\vel.exe
& $signTool verify /pa /all installer_output\VelSetup-x64.exe
```

The local certificate should be removed after testing if it is no longer needed:

```powershell
Get-ChildItem Cert:\CurrentUser\My | Where-Object Subject -Like '*Zyrndotio Local Test*' | Remove-Item
Get-ChildItem Cert:\CurrentUser\TrustedPublisher | Where-Object Subject -Like '*Zyrndotio Local Test*' | Remove-Item
Remove-Item "$HOME\vel-local-test.pfx", "$HOME\vel-local-test.cer"
```

## Production signing choices

For public Vel releases, use one consistent verified identity for every Windows release. The practical choices are:

| Option | Best fit | Important tradeoff |
|---|---|---|
| **Microsoft Artifact Signing** | CI-first release signing without managing a private PFX on GitHub | Requires Azure setup, identity validation, and service configuration; SmartScreen reputation still accumulates over time |
| **OV code-signing certificate** | Traditional publisher identity for downloadable installers | Requires organization or identity validation and secure private-key storage; initial SmartScreen warnings may still occur |
| **Microsoft Store distribution** | Consumer distribution where avoiding download SmartScreen prompts is the priority | Requires Store packaging, submission, and Microsoft distribution rules |

Purchase or enroll only through a certificate authority or Microsoft’s official service. Confirm that the certificate is for **Windows Authenticode code signing**, supports SHA-256, and can be used by your CI signing design. Do not buy an EV certificate solely to bypass SmartScreen; Microsoft’s current guidance says EV no longer guarantees that behavior.

During validation, use the legal identity that should appear in the certificate subject and publisher display. If **Zyrndotio** is a brand rather than a registered legal entity, verify the certificate authority’s subject-name requirements before paying.

## GitHub Actions integration

The Vel release workflow already contains optional signing steps. They are skipped safely when the signing secrets are absent, so unsigned development releases continue to build. To activate them:

1. Obtain a production Authenticode certificate or configure Microsoft Artifact Signing.
2. Export the certificate as a password-protected PFX if using a certificate-file workflow.
3. Base64-encode the PFX locally without committing it:

   ```powershell
   [Convert]::ToBase64String([IO.File]::ReadAllBytes('.\zyrndotio-code-signing.pfx')) |
     Set-Content .\zyrndotio-code-signing.pfx.b64
   ```

4. In the repository settings, open **Settings → Secrets and variables → Actions → New repository secret**.
5. Add these repository or protected-environment secrets:

   - `WINDOWS_CODESIGNING_CERT_BASE64`: base64 content of the PFX.
   - `WINDOWS_CODESIGNING_CERT_PASSWORD`: the PFX password.

6. Prefer a protected `release` environment with required reviewers for these secrets.
7. Push a release tag and inspect the Windows job. It signs and verifies both `vel.exe` and `VelSetup-x64.exe` using SHA-256 and an RFC 3161 timestamp.

The workflow never writes the certificate into the repository. It materializes the PFX only under the ephemeral runner’s temporary directory. Rotate the certificate and password if the private key may have been exposed.

## Verification after publication

On a Windows test machine:

```powershell
Get-AuthenticodeSignature .\VelSetup-x64.exe | Format-List *
Get-FileHash .\VelSetup-x64.exe -Algorithm SHA256
```

The signature status should be `Valid` for a certificate trusted by that machine, and the signer should identify the production publisher. Compare the hash with the release’s `SHA256SUMS.txt`. A valid signature proves integrity and publisher identity; it does not guarantee that SmartScreen will stop warning on the first download.

## References

- [Microsoft SignTool documentation](https://learn.microsoft.com/en-us/windows/win32/seccrypto/signtool)
- [Microsoft SmartScreen reputation guidance](https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/smartscreen-reputation)
- [GitHub Actions encrypted secrets](https://docs.github.com/actions/security-guides/using-secrets-in-github-actions)
- [Microsoft Artifact Signing](https://azure.microsoft.com/pricing/details/artifact-signing/)
