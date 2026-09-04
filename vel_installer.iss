[Setup]
AppName=Vel Programming Language
AppId={{A9E8D7C6-B5A4-4321-9F8E-7D6C5B4A3F2E}
AppVersion=0.2.0
AppPublisher=Zyrndotio
DefaultDirName={localappdata}\Programs\Vel
DefaultGroupName=Vel
PrivilegesRequired=lowest
OutputDir=installer_output
OutputBaseFilename=VelSetup-x64
Compression=lzma2
SolidCompression=yes
LicenseFile=LICENSE

; Vel is installed per-user so standard users do not need elevation.
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

; Inno Setup Modern Styles & Customizations
WizardStyle=modern
ChangesEnvironment=yes

[Files]
; Grab your freshly built binary from your MSBuild Release tree
Source: "build\Release\vel.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "assets\logo.png"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "docs\*"; DestDir: "{app}\docs"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "examples\*"; DestDir: "{app}\examples"; Flags: recursesubdirs createallsubdirs ignoreversion

[Registry]
; Append the binary directory to the current user's PATH without elevation.
Root: HKCU; Subkey: "Environment"; \
    ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}\bin"; \
    Check: NeedsAddPath('{app}\bin')

[Code]
const
  EnvironmentKey = 'Environment';

// 1. Installation: Prevent duplicate entries in the system PATH string
function NeedsAddPath(Param: string): boolean;
var
  OrigPath: string;
begin
  if RegQueryStringValue(HKCU, EnvironmentKey, 'Path', OrigPath) then
  begin
    Result := Pos(';' + Uppercase(Param) + ';', ';' + Uppercase(OrigPath) + ';') = 0;
  end
  else
  begin
    Result := True;
  end;
end;

// 2. Uninstallation: Programmatically target and slice out the Vel environment trace
procedure RemovePath(PathToRemove: string);
var
  OrigPath, CleanedPath: string;
  PosIndex: Integer;
begin
  if RegQueryStringValue(HKCU, EnvironmentKey, 'Path', OrigPath) then
  begin
    CleanedPath := OrigPath;
    PosIndex := Pos(';' + Uppercase(PathToRemove), Uppercase(CleanedPath));

    if PosIndex > 0 then
    begin
      Delete(CleanedPath, PosIndex, Length(PathToRemove) + 1);
      RegWriteExpandStringValue(HKCU, EnvironmentKey, 'Path', CleanedPath);
    end;
  end;
end;

// Native Event hook triggered right before uninstallation ends
procedure CurUninstallStepChanged(JustAfterAnUninstallStep: TUninstallStep);
begin
  if JustAfterAnUninstallStep = usPostUninstall then
  begin
    // Clean up the binary trace left inside the developer's registry
    RemovePath(ExpandConstant('{app}\bin'));
  end;
end;
