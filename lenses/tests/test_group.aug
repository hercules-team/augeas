module Test_group =

let conf = "bin:x:2:
audio:x:29:joe
avahi-autoipd:x:113:bill,martha
"

test Group.lns get conf =
   { "bin"
     { "password" = "x" }
     { "gid" = "2" }
     { "users" } }
   { "audio"
     { "password" = "x" }
     { "gid" = "29" }
     { "users" = "joe" } }
   { "avahi-autoipd"
     { "password" = "x" }
     { "gid" = "113" }
     { "users" = "bill,martha"} }
