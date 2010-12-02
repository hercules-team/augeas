module Test_access =

let conf = "+ : ALL : LOCAL
+ : root : localhost.localdomain
- : root : 127.0.0.1 .localdomain
"

test Access.lns get conf =
    { "access" = "+"
        { "user" = "ALL" }
        { "host" = "LOCAL" } }
    { "access" = "+"
        { "user" = "root" }
        { "host" = "localhost.localdomain" } }
    { "access" = "-"
        { "user" = "root" }
        { "host" = "127.0.0.1" }
        { "host" = ".localdomain" } }

test Access.lns put conf after
    insa "access" "access[last()]" ;
    set "access[last()]" "-" ;
    set "access[last()]/user" "ALL" ;
    set "access[last()]/host" "ALL"
 = "+ : ALL : LOCAL
+ : root : localhost.localdomain
- : root : 127.0.0.1 .localdomain
- : ALL : ALL
"
