[Setup]
AppName=OPL3GM VSTi
AppVerName=OPL3GM VSTi
DefaultDirName={pf}\OPL3GM
DefaultGroupName=OPL3GM
AllowNoIcons=yes
OutputBaseFilename=OPL3GM
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=X64 IA64
LicenseFile=license.txt

[Languages]
Name: en; MessagesFile: "compiler:Default.isl"
Name: ba; MessagesFile: "compiler:Languages\Basque.isl"
Name: br; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: ca; MessagesFile: "compiler:Languages\Catalan.isl"
Name: cz; MessagesFile: "compiler:Languages\Czech.isl"
Name: da; MessagesFile: "compiler:Languages\Danish.isl"
Name: nl; MessagesFile: "compiler:Languages\Dutch.isl"
Name: fi; MessagesFile: "compiler:Languages\Finnish.isl"
Name: fr; MessagesFile: "compiler:Languages\French.isl"
Name: de; MessagesFile: "compiler:Languages\German.isl"
Name: he; MessagesFile: "compiler:Languages\Hebrew.isl"
Name: hu; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: it; MessagesFile: "compiler:Languages\Italian.isl"
Name: ja; MessagesFile: "compiler:Languages\Japanese.isl"
Name: no; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: pl; MessagesFile: "compiler:Languages\Polish.isl"
Name: pt; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: ru; MessagesFile: "compiler:Languages\Russian.isl"
Name: se; MessagesFile: "compiler:Languages\SerbianLatin.isl"
Name: se2; MessagesFile: "compiler:Languages\SerbianCyrillic.isl"
Name: sl; MessagesFile: "compiler:Languages\Slovak.isl"
Name: sl2; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: sp; MessagesFile: "compiler:Languages\Spanish.isl"
Name: uk; MessagesFile: "compiler:Languages\Ukrainian.isl"

[Components]
Name: "Apogee32"; Description: "Apogee OPL3 (32-bit)"; Types: "full"
Name: "Apogee64"; Description: "Apogee OPL3 (64-bit)"; Types: "full"; Check: Is64BitInstallMode
Name: "Doom32"; Description: "Doom OPL3 (32-bit)"; Types: "full"
Name: "Doom64"; Description: "Doom OPL3 (64-bit)"; Types: "full"; Check: Is64BitInstallMode
Name: "W9x32"; Description: "Windows 9x OPL3 (32-bit)"; Types: "full"
Name: "W9x64"; Description: "Windows 9x OPL3 (64-bit)"; Types: "full"; Check: Is64BitInstallMode
Name: "ApogeeBanks"; Description: "Apogee Instrument Bank Files"; Types: "full"
Name: "DoomBanks"; Description: "Doom Instrument Bank Files"; Types: "full"
Name: "Docs"; Description: "Documentation"; Types: "full"

[Files]
Source: "OPL3Apogee.dll"; DestDir: {code:GetVSTDir_32}; Components: Apogee32; Flags: ignoreversion
Source: "OPL3Apogee_IA64.dll"; DestDir: {code:GetVSTDir_64}; Components: Apogee64; Check: IsIA64; Flags: ignoreversion
Source: "OPL3Apogee_X64.dll"; DestDir: {code:GetVSTDir_64}; Components: Apogee64; Check: IsX64; Flags: ignoreversion
Source: "OPL3Doom.dll"; DestDir: {code:GetVSTDir_32}; Components: Doom32; Flags: ignoreversion
Source: "OPL3Doom_IA64.dll"; DestDir: {code:GetVSTDir_64}; Components: Doom64; Check: IsIA64; Flags: ignoreversion
Source: "OPL3Doom_X64.dll"; DestDir: {code:GetVSTDir_64}; Components: Doom64; Check: IsX64; Flags: ignoreversion
Source: "OPL3W9x.dll"; DestDir: {code:GetVSTDir_32}; Components: W9x32; Flags: ignoreversion
Source: "OPL3W9x_IA64.dll"; DestDir: {code:GetVSTDir_64}; Components: W9x64; Check: IsIA64; Flags: ignoreversion
Source: "OPL3W9x_X64.dll"; DestDir: {code:GetVSTDir_64}; Components: W9x64; Check: IsX64; Flags: ignoreversion
Source: "Banks\Apogee\*"; DestDir: "{app}\Banks\Apogee"; Components: "ApogeeBanks"
Source: "Banks\Doom\*"; DestDir: "{app}\Banks\Doom"; Components: "DoomBanks"
Source: "license.txt"; DestDir: "{app}"; Components: "Docs"
Source: "readme.md"; DestDir: "{app}"; Components: "Docs"; DestName: "readme.txt"; Flags: isreadme

[Registry]
Root: HKCU; Subkey: "SOFTWARE\Datajake\OPL3GM"; ValueName: "ApogeePatchDir"; ValueType: String; ValueData: "{app}\Banks\Apogee"; Components: "ApogeeBanks"; Flags: createvalueifdoesntexist
Root: HKCU; Subkey: "SOFTWARE\Datajake\OPL3GM"; ValueName: "DoomPatchDir"; ValueType: String; ValueData: "{app}\Banks\Doom"; Components: "DoomBanks"; Flags: createvalueifdoesntexist

[Icons]
Name: "{group}\Open Apogee Banks Directory"; Filename: "{app}\Banks\Apogee"; Components: "ApogeeBanks"
Name: "{group}\Open Doom Banks Directory"; Filename: "{app}\Banks\Doom"; Components: "DoomBanks"
Name: "{group}\License"; Filename: "{app}\license.txt"; Components: "Docs"
Name: "{group}\Readme"; Filename: "{app}\readme.txt"; Components: "Docs"
Name: "{group}\Uninstall"; Filename: "{uninstallexe}"

[Messages]
SelectDirLabel3=The documentation and instrument banks will be installed in the following folder.

[Code]
function IsX64: Boolean;
begin
  Result := Is64BitInstallMode and (ProcessorArchitecture = paX64);
end;

function IsIA64: Boolean;
begin
  Result := Is64BitInstallMode and (ProcessorArchitecture = paIA64);
end;

var
  VSTDirPage_32: TInputDirWizardPage;
  VSTDirPage_64: TInputDirWizardPage;

procedure InitializeWizard;
begin
  if IsWin64 then begin
    VSTDirPage_64 := CreateInputDirPage(wpSelectDir,
    'Confirm 64-Bit VST Plug-in Directory', '',
    'Select the folder in which setup should install the 64-bit VST Plug-ins, then click Next.',
    False, '');
    VSTDirPage_64.Add('');
    VSTDirPage_64.Values[0] := ExpandConstant('{reg:HKLM\SOFTWARE\VST,VSTPluginsPath|{pf}\Steinberg\VSTPlugins}\');

    VSTDirPage_32 := CreateInputDirPage(wpSelectDir,
      'Confirm 32-Bit VST Plug-in Directory', '',
      'Select the folder in which setup should install the 32-bit VST Plug-ins, then click Next.',
      False, '');
    VSTDirPage_32.Add('');
    VSTDirPage_32.Values[0] := ExpandConstant('{reg:HKLM\SOFTWARE\WOW6432NODE\VST,VSTPluginsPath|{pf32}\Steinberg\VSTPlugins}\');
  end else begin
    VSTDirPage_32 := CreateInputDirPage(wpSelectDir,
      'Confirm 32-Bit VST Plug-in Directory', '',
      'Select the folder in which setup should install the 32-bit VST Plug-ins, then click Next.',
      False, '');
    VSTDirPage_32.Add('');
    VSTDirPage_32.Values[0] := ExpandConstant('{reg:HKLM\SOFTWARE\VST,VSTPluginsPath|{pf}\Steinberg\VSTPlugins}\');
  end;
end;

function GetVSTDir_32(Param: String): String;
begin
  Result := VSTDirPage_32.Values[0]
end;

function GetVSTDir_64(Param: String): String;
begin
  Result := VSTDirPage_64.Values[0]
end;
