module Test_avahi =

   let conf = "
[server]
host-name=web
domain=example.com

[wide-area]
enable-wide-area=yes
"

   test Avahi.lns get conf =
      {}
      { "server"
         { "host-name" = "web" }
         { "domain" = "example.com" }
         {} }
      { "wide-area"
         { "enable-wide-area" = "yes" } }

    test Avahi.lns put conf after
       set "server/use-ipv4" "yes";
       set "server/clients-max" "4096"
    = "
[server]
host-name=web
domain=example.com

use-ipv4=yes
clients-max=4096
[wide-area]
enable-wide-area=yes
"

