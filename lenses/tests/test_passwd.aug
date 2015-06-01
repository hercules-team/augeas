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

test Passwd.lns get "+::::::\n" =
  { "@nisdefault"
      { "password" = "" }
      { "uid" = "" }
      { "gid" = "" }
      { "name" }
      { "home" }
      { "shell" } }

test Passwd.lns get "+::::::/sbin/nologin\n" =
  { "@nisdefault"
    { "password" = "" }
    { "uid" = "" }
    { "gid" = "" }
    { "name" }
    { "home" }
    { "shell" = "/sbin/nologin" } }

test Passwd.lns get "+:*:0:0:::\n" =
  { "@nisdefault"
    { "password" = "*" }
    { "uid" = "0" }
    { "gid" = "0" }
    { "name" }
    { "home" }
    { "shell" } }

(* NIS entries with overrides, ticket #339 *)
test Passwd.lns get "+@bob:::::/home/bob:/bin/bash\n" =
 { "@nis" = "bob"
   { "home" = "/home/bob" }
   { "shell" = "/bin/bash" } }

(* NIS user entries *)
test Passwd.lns get "+bob::::::\n" =
 { "@+nisuser" = "bob" }

test Passwd.lns get "+bob::::User Comment:/home/bob:/bin/bash\n" =
 { "@+nisuser" = "bob"
   { "name" = "User Comment" }
   { "home" = "/home/bob" }
   { "shell" = "/bin/bash" } }

test Passwd.lns put "+bob::::::\n" after
  set "@+nisuser" "alice"
= "+alice::::::\n"

test Passwd.lns put "+bob::::::\n" after
  set "@+nisuser/name" "User Comment";
  set "@+nisuser/home" "/home/bob";
  set "@+nisuser/shell" "/bin/bash"
= "+bob::::User Comment:/home/bob:/bin/bash\n"

test Passwd.lns get "-bob::::::\n" =
 { "@-nisuser" = "bob" }

test Passwd.lns put "-bob::::::\n" after
  set "@-nisuser" "alice"
= "-alice::::::\n"
