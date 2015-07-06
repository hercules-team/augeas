module Test_MasterPasswd =

let conf = "root:*:0:0:daemon:0:0:Charlie &:/root:/bin/ksh
sshd:*:27:27::0:0:sshd privsep:/var/empty:/sbin/nologin
_portmap:*:28:28::0:0:portmap:/var/empty:/sbin/nologin
test:x:1000:1000:ldap:1434329080:1434933880:Test User,,,:/home/test:/bin/bash
"

test MasterPasswd.lns get conf =
   { "root"
     { "password" = "*" }
     { "uid" = "0" }
     { "gid" = "0" }
     { "class" = "daemon" }
     { "change_date" = "0" }
     { "expire_date" = "0" }
     { "name" = "Charlie &" }
     { "home" = "/root" }
     { "shell" = "/bin/ksh" } }
   { "sshd"
     { "password" = "*" }
     { "uid" = "27" }
     { "gid" = "27" }
     { "class" }
     { "change_date" = "0" }
     { "expire_date" = "0" }
     { "name" = "sshd privsep" }
     { "home" = "/var/empty" }
     { "shell" = "/sbin/nologin" } }
   { "_portmap"
     { "password" = "*" }
     { "uid" = "28" }
     { "gid" = "28" }
     { "class" }
     { "change_date" = "0" }
     { "expire_date" = "0" }
     { "name" = "portmap" }
     { "home" = "/var/empty" }
     { "shell" = "/sbin/nologin" } }
   { "test"
     { "password" = "x" }
     { "uid" = "1000" }
     { "gid" = "1000" }
     { "class" = "ldap" }
     { "change_date" = "1434329080" }
     { "expire_date" = "1434933880" }
     { "name" = "Test User,,," }
     { "home" = "/home/test" }
     { "shell" = "/bin/bash" } }

(* Popular on Solaris *)
test MasterPasswd.lns get "+@some-nis-group:::::::::\n" =
  { "@nis" = "some-nis-group" }

test MasterPasswd.lns get "+\n" =
  { "@nisdefault" }

test MasterPasswd.lns get "+:::::::::\n" =
  { "@nisdefault"
      { "password" = "" }
      { "uid" = "" }
      { "gid" = "" }
      { "class" }
      { "change_date" = "" }
      { "expire_date" = "" }
      { "name" }
      { "home" }
      { "shell" } }

test MasterPasswd.lns get "+:::::::::/sbin/nologin\n" =
  { "@nisdefault"
    { "password" = "" }
    { "uid" = "" }
    { "gid" = "" }
    { "class" }
    { "change_date" = "" }
    { "expire_date" = "" }
    { "name" }
    { "home" }
    { "shell" = "/sbin/nologin" } }

test MasterPasswd.lns get "+:*:::ldap:::::\n" =
  { "@nisdefault"
    { "password" = "*" }
    { "uid" = "" }
    { "gid" = "" }
    { "class" = "ldap" }
    { "change_date" = "" }
    { "expire_date" = "" }
    { "name" }
    { "home" }
    { "shell" } }

(* NIS entries with overrides, ticket #339 *)
test MasterPasswd.lns get "+@bob::::::::/home/bob:/bin/bash\n" =
 { "@nis" = "bob"
   { "home" = "/home/bob" }
   { "shell" = "/bin/bash" } }

(* NIS user entries *)
test MasterPasswd.lns get "+bob:::::::::\n" =
 { "@+nisuser" = "bob" }

test MasterPasswd.lns get "+bob:::::::User Comment:/home/bob:/bin/bash\n" =
 { "@+nisuser" = "bob"
   { "name" = "User Comment" }
   { "home" = "/home/bob" }
   { "shell" = "/bin/bash" } }

test MasterPasswd.lns put "+bob:::::::::\n" after
  set "@+nisuser" "alice"
= "+alice:::::::::\n"

test MasterPasswd.lns put "+bob:::::::::\n" after
  set "@+nisuser/name" "User Comment";
  set "@+nisuser/home" "/home/bob";
  set "@+nisuser/shell" "/bin/bash"
= "+bob:::::::User Comment:/home/bob:/bin/bash\n"

test MasterPasswd.lns get "-bob:::::::::\n" =
 { "@-nisuser" = "bob" }

test MasterPasswd.lns put "-bob:::::::::\n" after
  set "@-nisuser" "alice"
= "-alice:::::::::\n"
