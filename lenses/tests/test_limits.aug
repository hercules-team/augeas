module Test_limits =

let conf = "@audio - rtprio 99
ftp hard nproc /ftp
* soft core 0
"

test Limits.lns get conf =
   { "@audio"
     { "type"  = "-" }
     { "item"  = "rtprio" }
     { "value" = "99" } }
   { "ftp"
     { "type" = "hard" }
     { "item" = "nproc" }
     { "value" = "/ftp" } }
   { "*"
     { "type" = "soft" }
     { "item" = "core" }
     { "value" = "0" } }
