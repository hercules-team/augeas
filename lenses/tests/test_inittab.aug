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
    { "id"
          { "runlevels" = "5" }
          { "action" = "initdefault" }
          { "process" = "" } }
    { "#comment" = "System initialization." }
    { "si"
          { "runlevels" = "" }
          { "action" = "sysinit" }
          { "process" = "/etc/rc.d/rc.sysinit" } }
    { }
    { "#comment" = "Trap CTRL-ALT-DELETE" }
    { "ca"
          { "runlevels" = "" }
          { "action" = "ctrlaltdel" }
          { "process" = "/sbin/shutdown -t3 -r now" } }
    { }
    { "l0"
          { "runlevels" = "0" }
          { "action" = "wait" }
          { "process" = "/etc/rc.d/rc 0" } }

  test Inittab.lns put simple after rm "/id/process" = *

  test Inittab.lns put simple after set "/id/runlevels" "3" =
    "id:3:initdefault:\n"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
