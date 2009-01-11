module Test_inittab =

  let simple  = "id:5:initdefault:\n"

  let inittab = "id:5:initdefault:
# System initialization.
si::sysinit:/etc/rc.d/rc.sysinit
# Trap CTRL-ALT-DELETE
ca::ctrlaltdel:/sbin/shutdown -t3 -r now
l0:0:wait:/etc/rc.d/rc 0
"

 test Inittab.lns get inittab =
    { "1" { "id" = "id" }
          { "runlevels" = "5" }
          { "action" = "initdefault" }
          { "process" = "" } }
    {}
    { "2" { "id" = "si" }
          { "runlevels" = "" }
          { "action" = "sysinit" }
          { "process" = "/etc/rc.d/rc.sysinit" } }
    {}
    { "3" { "id" = "ca" }
          { "runlevels" = "" }
          { "action" = "ctrlaltdel" }
          { "process" = "/sbin/shutdown -t3 -r now" } }
    { "4" { "id" = "l0" }
          { "runlevels" = "0" }
          { "action" = "wait" }
          { "process" = "/etc/rc.d/rc 0" } }

  test Inittab.lns put simple after rm "/1/process" = *

  test Inittab.lns put simple after set "/1/runlevels" "3" =
    "id:3:initdefault:\n"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
