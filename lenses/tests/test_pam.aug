module Test_pam =

  let example = "#%PAM-1.0
auth [user_unknown=ignore success=ok ignore=ignore default=bad] pam_securetty.so
session    optional     pam_keyinit.so force revoke
"

  test Pam.lns get example = 
    {}
    { "1" { "type" = "auth" }
          { "control" = "[user_unknown=ignore success=ok ignore=ignore default=bad]" }
          { "module" = "pam_securetty.so" } }
    { "2" { "type" = "session" }
          { "control" = "optional" }
          { "module" = "pam_keyinit.so" }
          { "opts" = "force revoke" } }

  test Pam.lns put example after
    set "1/control" "requisite"
  = "#%PAM-1.0
auth requisite pam_securetty.so
session    optional     pam_keyinit.so force revoke
"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
