module Test_AuthselectPam =

let example ="auth        required                                     pam_env.so
auth        required                                     pam_faildelay.so delay=2000000
auth        required                                     pam_faillock.so preauth silent                         {include if \"with-faillock\"}
auth        required                                     pam_u2f.so cue {if not \"without-pam-u2f-nouserok\":nouserok} {include if \"with-pam-u2f-2fa\"}
"

test AuthselectPam.lns get example =
  { "1"
    { "type" = "auth" }
    { "control" = "required" }
    { "module" = "pam_env.so" } }
  { "2"
    { "type" = "auth" }
    { "control" = "required" }
    { "module"  = "pam_faildelay.so" }
    { "argument" = "delay=2000000" } }
  { "3"
    { "type" = "auth" }
    { "control" = "required" }
    { "module"  = "pam_faillock.so" }
    { "argument" = "preauth" }
    { "argument" = "silent" }
    { "authselect_conditional" = "include if"
      { "feature" = "with-faillock" } } }
  { "4"
    { "type" = "auth" }
    { "control" = "required" }
    { "module" = "pam_u2f.so" }
    { "argument" = "cue" }
    { "authselect_conditional" = "if"
      { "not" }
      { "feature" = "without-pam-u2f-nouserok" }
      { "on_true" = "nouserok" } }
    { "authselect_conditional" = "include if"
      { "feature" = "with-pam-u2f-2fa" } } }
