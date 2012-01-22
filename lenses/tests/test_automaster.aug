module Test_automaster =

  let example = "#
# Sample auto.master file
#

/-        auto.data
/net    -hosts ro
/misc   /etc/auto.misc
/home     /etc/auto.home
/home     ldap:example.com:ou=auto.home,dc=example,dc=com 
/mnt      yp:mnt.map -strict,-Dfoo=bar,uid=1000
/mnt      yp,sun:mnt.map

+dir:/etc/auto.master.d
+ auto.master
"

  test Automaster.lns get example =
    { }
    { "#comment" = "Sample auto.master file" }
    { }
    { }
    { "1" = "/-"
        { "map" = "auto.data" } }
    { "2" = "/net"
        { "map" = "-hosts" }
        { "opt" = "ro" } }
    { "3" = "/misc"
        { "map" = "/etc/auto.misc" } }
    { "4" = "/home"
        { "map" = "/etc/auto.home" } }
    { "5" = "/home"
        { "type" = "ldap" }
        { "host" = "example.com" }
        { "map" = "ou=auto.home,dc=example,dc=com" } }
    { "6" = "/mnt"
        { "type" = "yp" }
        { "map" = "mnt.map" }
        { "opt" = "-strict" }
        { "opt" = "-Dfoo"
            { "value" = "bar" } }
        { "opt" = "uid"
            { "value" = "1000" } } }
    { "7" = "/mnt"
        { "type" = "yp" }
        { "format" = "sun" }
        { "map" = "mnt.map" } }
    { }
    { "8" = "+"
        { "type" = "dir" }
        { "map" = "/etc/auto.master.d" } }
    { "9" = "+"
        { "map" = "auto.master" } }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
