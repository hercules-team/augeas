module Test_wine =

let s1 = "WINE REGISTRY Version 2
;; All keys relative to \\Machine

[Software\\Borland\\Database Engine\\Settings\\SYSTEM\\INIT] 1255960431
\"SHAREDMEMLOCATION\"=\"9000\"

[Software\\Classes\\.gif] 1255960430
@=\"giffile\"
\"Content Type\"=\"image/gif\"

[Software\\Classes\\CLSID\\{083863F1-70DE-11D0-BD40-00A0C911CE86}\\Instance\\{1B544C20-FD0B-11CE-8C63-00AA0044B51E}] 1255960430
\"CLSID\"=\"{1B544C20-FD0B-11CE-8C63-00AA0044B51E}\"
\"FilterData\"=hex:02,00,00,00,00,00,60,00,02,00,00,00,00,00,00,00,30,70,69,33,\
  9f,53,00,20,af,0b,a7,70,76,69,64,73,00,00,10,00,80,00,00,aa,00,38,9b,71,00,\
  00,00,00,00,00,00,00,00,00,00,00,00,00,00,00
\"FriendlyName\"=\"AVI Splitter\"

[Software\\Classes\\CLSID\\{0AFACED1-E828-11D1-9187-B532F1E9575D}\\ShellFolder] 1255960429
\"Attributes\"=dword:60010000
\"CallForAttributes\"=dword:f0000000
"

test Wine.lns get s1 =
  { "registry" = "WINE REGISTRY" }
  { "version" = "2" }
  { "#comment" = "All keys relative to \Machine" }
  { }
  { "section" = "Software\Borland\Database Engine\Settings\SYSTEM\INIT"
    { "timestamp" = "1255960431" }
    { "entry"
      { "key" = "SHAREDMEMLOCATION" }
      { "value" = "9000" } }
    { } }
  { "section" = "Software\Classes\.gif"
    { "timestamp" = "1255960430" }
    { "anon" { "value" = "giffile" } }
    { "entry"
      { "key" = "Content Type" }
      { "value" = "image/gif" } }
    { } }
  { "section" = "Software\Classes\CLSID\{083863F1-70DE-11D0-BD40-00A0C911CE86}\Instance\{1B544C20-FD0B-11CE-8C63-00AA0044B51E}"
    { "timestamp" = "1255960430" }
    { "entry"
      { "key" = "CLSID" }
      { "value" = "{1B544C20-FD0B-11CE-8C63-00AA0044B51E}" } }
    { "entry"
      { "key" = "FilterData" }
      { "type" = "hex" }
      { "value" = "02,00,00,00,00,00,60,00,02,00,00,00,00,00,00,00,30,70,69,33,\
  9f,53,00,20,af,0b,a7,70,76,69,64,73,00,00,10,00,80,00,00,aa,00,38,9b,71,00,\
  00,00,00,00,00,00,00,00,00,00,00,00,00,00,00" } }
    { "entry"
      { "key" = "FriendlyName" }
      { "value" = "AVI Splitter" } }
    { } }
  { "section" = "Software\Classes\CLSID\{0AFACED1-E828-11D1-9187-B532F1E9575D}\ShellFolder"
    { "timestamp" = "1255960429" }
    { "entry"
      { "key" = "Attributes" }
      { "type" = "dword" }
      { "value" = "60010000" } }
    { "entry"
      { "key" = "CallForAttributes" }
      { "type" = "dword" }
      { "value" = "f0000000" } } }

(* The weird 'str(2)' type *)
let s2 = "[Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders] 1248768928
\"AppData\"=str(2):\"%USERPROFILE%\\Application Data\"
\"Cache\"=str(2):\"%USERPROFILE%\\Local Settings\\Temporary Internet Files\"
\"Cookies\"=str(2):\"%USERPROFILE%\\Cookies\"
\"Desktop\"=str(2):\"%USERPROFILE%\\Desktop\"
"

test Wine.section get s2 =
  { "section" =
    "Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders"
    { "timestamp" = "1248768928" }
    { "entry"
      { "key" = "AppData" }
      { "type" = "str(2)" }
      { "value" = "%USERPROFILE%\Application Data" } }
    { "entry"
      { "key" = "Cache" }
      { "type" = "str(2)" }
      { "value" = "%USERPROFILE%\Local Settings\Temporary Internet Files" } }
    { "entry"
      { "key" = "Cookies" }
      { "type" = "str(2)" }
      { "value" = "%USERPROFILE%\Cookies" } }
    { "entry"
      { "key" = "Desktop" }
      { "type" = "str(2)" }
      { "value" = "%USERPROFILE%\Desktop" } } }

(* Quoted doublequotes embedded in the string *)
let s3 = "[Software\\Classes\\CLSID\\Shell\\OpenHomePage\\Command] 1248768931
@=\"\\\"C:\\Program Files\\Internet Explorer\\iexplore.exe\\\"\"\n"

test Wine.section get s3 =
  { "section" = "Software\Classes\CLSID\Shell\OpenHomePage\Command"
    { "timestamp" = "1248768931" }
    { "anon"
      { "value" = "\\\"C:\Program Files\Internet Explorer\iexplore.exe\\\"" } } }

(* There's a str(7) type, too *)
let s4 = "[Software\\Microsoft\\Cryptography\\OID\\EncodingType 1\\CertDllVerifyRevocation\\DEFAULT] 1248768928
\"Dll\"=str(7):\"cryptnet.dll\0\"\n"

test Wine.section get s4 =
  { "section" = "Software\Microsoft\Cryptography\OID\EncodingType 1\CertDllVerifyRevocation\DEFAULT"
    { "timestamp" = "1248768928" }
    { "entry"
      { "key" = "Dll" }
      { "type" = "str(7)" }
      { "value" = "cryptnet.dll\0" } } }

(* The Windows Registry Editor header *)
test Wine.header get "Windows Registry Editor Version 5.00\n" =
  { "registry" = "Windows Registry Editor" }
  { "version" = "5.00" }

(* The type hex(7) *)
let s5 = "[HKEY_LOCAL_MACHINE\SOFTWARE\ADFS]
\"Patches\"=hex(7):25,00,77,00,69,00,6e,00,64,00,69,00,72,00,25,00,5c,00,61,00,\
  64,00,66,00,73,00,73,00,70,00,32,00,2e,00,6d,00,73,00,70,00,\
  00,00,00,00,f8
"
test Wine.section get s5 =
  { "section" = "HKEY_LOCAL_MACHINE\SOFTWARE\ADFS"
    { "entry"
      { "key" = "Patches" }
      { "type" = "hex(7)" }
      { "value" = "25,00,77,00,69,00,6e,00,64,00,69,00,72,00,25,00,5c,00,61,00,\
  64,00,66,00,73,00,73,00,70,00,32,00,2e,00,6d,00,73,00,70,00,\
  00,00,00,00,f8" } } }

(* Test DOS line endings *)
let s6 = "Windows Registry Editor Version 5.00\r\n\r
[HKEY_LOCAL_MACHINE\SOFTWARE\C07ft5Y\WinXP]\r
\"@\"=\"\"\r
\r
[HKEY_LOCAL_MACHINE\SOFTWARE\Classes]\r\n"

test Wine.lns get s6 =
  { "registry" = "Windows Registry Editor" }
  { "version" = "5.00" }
  { }
  { "section" = "HKEY_LOCAL_MACHINE\SOFTWARE\C07ft5Y\WinXP"
    { "anon" { "value" = "" } }
    { } }
  { "section" = "HKEY_LOCAL_MACHINE\SOFTWARE\Classes" }

(* Keys can contain '/' *)
let s7 =
"\"application/vnd.ms-xpsdocument\"=\"{c18d5e87-12b4-46a3-ae40-67cf39bc6758}\"\n"

test Wine.entry get s7 =
  { "entry"
    { "key" = "application/vnd.ms-xpsdocument" }
    { "value" = "{c18d5e87-12b4-46a3-ae40-67cf39bc6758}" } }
