module Test_pamconf =

  let example = "# Authentication management
#
# login service (explicit because of pam_dial_auth)
#
login   auth requisite      pam_authtok_get.so.1
login   auth required       pam_dhkeys.so.1 arg

other   session required    pam_unix_session.so.1
"

  test PamConf.lns get example =
    { "#comment" = "Authentication management" }
    { }
    { "#comment" = "login service (explicit because of pam_dial_auth)" }
    { }
    { "1" { "service" = "login" }
          { "type" = "auth" }
          { "control" = "requisite" }
          { "module" = "pam_authtok_get.so.1" } }
    { "2" { "service" = "login" }
          { "type" = "auth" }
          { "control" = "required" }
          { "module" = "pam_dhkeys.so.1" }
          { "argument" = "arg" } }
    { }
    { "3" { "service" = "other" }
          { "type" = "session" }
          { "control" = "required" }
          { "module" = "pam_unix_session.so.1" } }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
