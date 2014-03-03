module Test_group =

let conf = "bin:x:2:
audio:x:29:joe
avahi-autoipd:!:113:bill,martha
"

test Group.lns get conf =
   { "bin"
     { "password" = "x" }
     { "gid" = "2" } }
   { "audio"
     { "password" = "x" }
     { "gid" = "29" }
     { "user" = "joe" } }
   { "avahi-autoipd"
     { "password" = "!" }
     { "gid" = "113" }
     { "user" = "bill"}
     { "user" = "martha"} }

(* Password field can be empty *)
test Group.lns get "root::0:root\n" =
  { "root"
    { "password" = "" }
    { "gid" = "0" }
    { "user" = "root" } }

(* Password field can be disabled by ! or * *)
test Group.lns get "testgrp:!:0:testusr\n" =
  { "testgrp"
    { "password" = "!" }
    { "gid" = "0" }
    { "user" = "testusr" } }

test Group.lns get "testgrp:*:0:testusr\n" =
  { "testgrp"
    { "password" = "*" }
    { "gid" = "0" }
    { "user" = "testusr" } }

(* NIS defaults *)
test Group.lns get "+\n" =
  { "@nisdefault" }

test Group.lns get "+:::\n" =
  { "@nisdefault"
    { "password" = "" }
    { "gid" = "" } }
