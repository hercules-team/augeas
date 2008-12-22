module Test_webmin =

let conf = "port=10000
realm=Webmin Server
denyfile=\.pl$
"

test Webmin.lns get conf =
   { "port" = "10000" }
   { "realm" = "Webmin Server" }
   { "denyfile" = "\.pl$" }
