module Test_Passwd =

let conf = "root:x:0:0:root:/root:/bin/bash
libuuid:x:100:101::/var/lib/libuuid:/bin/sh
free:x:1000:1000:Free Ekanayaka,,,:/home/free:/bin/bash
root:*:0:0:Charlie &:/root:/bin/csh
"

test Passwd.lns get conf =
   { "root"
     { "password" = "x" }
     { "uid" = "0" }
     { "gid" = "0" }
     { "name" = "root" }
     { "home" = "/root" }
     { "shell" = "/bin/bash" } }
   { "libuuid"
     { "password" = "x" }
     { "uid" = "100" }
     { "gid" = "101" }
     { "name" }
     { "home" = "/var/lib/libuuid" }
     { "shell" = "/bin/sh" } }
   { "free"
     { "password" = "x" }
     { "uid" = "1000" }
     { "gid" = "1000" }
     { "name" = "Free Ekanayaka,,," }
     { "home" = "/home/free" }
     { "shell" = "/bin/bash" } }
   { "root"
     { "password" = "*" }
     { "uid" = "0" }
     { "gid" = "0" }
     { "name" = "Charlie &" }
     { "home" = "/root" }
     { "shell" = "/bin/csh" } }

(* Popular on Solaris *)
test Passwd.lns get "+@some-nis-group::::::\n" =
  { "@nis" = "some-nis-group" }

test Passwd.lns get "+\n" =
  { "@nisdefault" }

test Passwd.lns get "+::::::/sbin/nologin\n" =
  { "@nisdefault"
    { "password" = "" }
    { "uid" = "" }
    { "gid" = "" }
    { "name" }
    { "home" }
    { "shell" = "/sbin/nologin" } }

(* NIS entries with overrides, ticket #339 *)
test Passwd.lns get "+@bob:::::/home/bob:/bin/bash\n" =
 { "@nis" = "bob"
   { "home" = "/home/bob" }
   { "shell" = "/bin/bash" } }
