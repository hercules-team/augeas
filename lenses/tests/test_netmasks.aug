(* Test for netmasks lens *)
module Test_netmasks =

  let conf = "# The netmasks file associates Internet Protocol (IP) address
# masks with IP network numbers.
#

192.168.1.0	255.255.255.0
10.0.0.0 255.0.0.0
"

  test Netmasks.lns get conf =
    { "#comment" = "The netmasks file associates Internet Protocol (IP) address" }
    { "#comment" = "masks with IP network numbers." }
    { }
    { }
    { "1"
        { "network" = "192.168.1.0" }
        { "netmask" = "255.255.255.0" } }
    { "2"
        { "network" = "10.0.0.0" }
        { "netmask" = "255.0.0.0" } }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
