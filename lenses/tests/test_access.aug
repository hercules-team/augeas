module Test_access =

let conf = "+ : ALL : LOCAL
+ : root : localhost.localdomain
- : root : 127.0.0.1 .localdomain
+ : root @admins : cron crond :0 tty1 tty2 tty3 tty4 tty5 tty6
# IP v6 support
+ : john foo : 2001:4ca0:0:101::1 2001:4ca0:0:101::/64
# Except
+ : ALL EXCEPT john @wheel : ALL EXCEPT LOCAL .win.tue.nl
"

test Access.lns get conf =
    { "access" = "+"
        { "user" = "ALL" }
        { "origin" = "LOCAL" } }
    { "access" = "+"
        { "user" = "root" }
        { "origin" = "localhost.localdomain" } }
    { "access" = "-"
        { "user" = "root" }
        { "origin" = "127.0.0.1" }
        { "origin" = ".localdomain" } }
    { "access" = "+"
        { "user" = "root" }
        { "netgroup" = "admins" }
        { "origin" = "cron" }
        { "origin" = "crond" }
        { "origin" = ":0" }
        { "origin" = "tty1" }
        { "origin" = "tty2" }
        { "origin" = "tty3" }
        { "origin" = "tty4" }
        { "origin" = "tty5" }
        { "origin" = "tty6" } }
    { "#comment" = "IP v6 support" }
    { "access" = "+"
        { "user" = "john" }
        { "user" = "foo" }
        { "origin" = "2001:4ca0:0:101::1" }
        { "origin" = "2001:4ca0:0:101::/64" } }
    { "#comment" = "Except" }
    { "access" = "+"
        { "user" = "ALL" }
        { "except"
           { "user" = "john" }
           { "netgroup" = "wheel" } }
        { "origin" = "ALL" }
        { "except"
           { "origin" = "LOCAL" }
           { "origin" = ".win.tue.nl" } } }

test Access.lns put conf after
    insa "access" "access[last()]" ;
    set "access[last()]" "-" ;
    set "access[last()]/user" "ALL" ;
    set "access[last()]/origin" "ALL"
 = "+ : ALL : LOCAL
+ : root : localhost.localdomain
- : root : 127.0.0.1 .localdomain
+ : root @admins : cron crond :0 tty1 tty2 tty3 tty4 tty5 tty6
# IP v6 support
+ : john foo : 2001:4ca0:0:101::1 2001:4ca0:0:101::/64
# Except
+ : ALL EXCEPT john @wheel : ALL EXCEPT LOCAL .win.tue.nl
- : ALL : ALL
"
