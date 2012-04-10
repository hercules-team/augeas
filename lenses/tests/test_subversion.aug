(*
Module: Test_Subversion
  Provides unit tests and examples for the <Subversion> lens.
*)

module Test_Subversion =

(* Variable: conf *)
let conf = "# This file configures various client-side behaviors.
[auth]
password-stores = gnome-keyring,kwallet
store-passwords = no
store-auth-creds = no

[helpers]
editor-cmd = /usr/bin/vim
diff-cmd = /usr/bin/diff
diff3-cmd = /usr/bin/diff3
diff3-has-program-arg = yes

[tunnels]
ssh = $SVN_SSH ssh -o ControlMaster=no
rsh = /path/to/rsh -l myusername

[miscellany]
global-ignores = *.o *.lo *.la *.al .libs *.so *.so.[0-9]* *.a *.pyc *.pyo
   *.rej *~ #*# .#* .*.swp .DS_Store
# Set log-encoding to the default encoding for log messages
log-encoding = latin1
use-commit-times = yes
no-unlock = yes
mime-types-file = /path/to/mime.types
preserved-conflict-file-exts = doc ppt xls od?
enable-auto-props = yes
interactive-conflicts = no

[auto-props]
*.c = svn:eol-style=native
*.cpp = svn:eol-style=native
*.h = svn:eol-style=native
*.dsp = svn:eol-style=CRLF
*.dsw = svn:eol-style=CRLF
*.sh = svn:eol-style=native;svn:executable
*.txt = svn:eol-style=native
*.png = svn:mime-type=image/png
*.jpg = svn:mime-type=image/jpeg
Makefile = svn:eol-style=native
"

(* Test: Subversion.lns *)
test Subversion.lns get conf =
{ "#comment" = "This file configures various client-side behaviors." }
  { "auth"
    { "password-stores"
      { "1" = "gnome-keyring" }
      { "2" = "kwallet" } }
    { "store-passwords" = "no" }
    { "store-auth-creds" = "no" }
    {  }
  }
  { "helpers"
    { "editor-cmd" = "/usr/bin/vim" }
    { "diff-cmd" = "/usr/bin/diff" }
    { "diff3-cmd" = "/usr/bin/diff3" }
    { "diff3-has-program-arg" = "yes" }
    {  }
  }
  { "tunnels"
    { "ssh" = "$SVN_SSH ssh -o ControlMaster=no" }
    { "rsh" = "/path/to/rsh -l myusername" }
    {  }
  }
  { "miscellany"
    { "global-ignores"
      { "1" = "*.o" }
      { "2" = "*.lo" }
      { "3" = "*.la" }
      { "4" = "*.al" }
      { "5" = ".libs" }
      { "6" = "*.so" }
      { "7" = "*.so.[0-9]*" }
      { "8" = "*.a" }
      { "9" = "*.pyc" }
      { "10" = "*.pyo" }
      { "11" = "*.rej" }
      { "12" = "*~" }
      { "13" = "#*#" }
      { "14" = ".#*" }
      { "15" = ".*.swp" }
      { "16" = ".DS_Store" } }
    { "#comment" = "Set log-encoding to the default encoding for log messages" }
    { "log-encoding" = "latin1" }
    { "use-commit-times" = "yes" }
    { "no-unlock" = "yes" }
    { "mime-types-file" = "/path/to/mime.types" }
    { "preserved-conflict-file-exts"
      { "1" = "doc" }
      { "2" = "ppt" }
      { "3" = "xls" }
      { "4" = "od?" } }
    { "enable-auto-props" = "yes" }
    { "interactive-conflicts" = "no" }
    {  }
  }
  { "auto-props"
    { "*.c" = "svn:eol-style=native" }
    { "*.cpp" = "svn:eol-style=native" }
    { "*.h" = "svn:eol-style=native" }
    { "*.dsp" = "svn:eol-style=CRLF" }
    { "*.dsw" = "svn:eol-style=CRLF" }
    { "*.sh" = "svn:eol-style=native;svn:executable" }
    { "*.txt" = "svn:eol-style=native" }
    { "*.png" = "svn:mime-type=image/png" }
    { "*.jpg" = "svn:mime-type=image/jpeg" }
    { "Makefile" = "svn:eol-style=native" }
  }


