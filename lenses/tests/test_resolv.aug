module Test_resolv =

   let conf = "# Sample resolv.conf
nameserver 192.168.0.3
nameserver ff02::1
domain mynet.com
search mynet.com anotherorg.net

# A sortlist now
sortlist 130.155.160.0/255.255.240.0 130.155.0.0

options ndots:3 debug timeout:2
options no-ip6-dotint
"

test Resolv.lns get conf =
   { "#comment" = "Sample resolv.conf" }
   { "nameserver" = "192.168.0.3" }
   { "nameserver" = "ff02::1" }
   { "domain" = "mynet.com" }
   { "search"
        { "domain" = "mynet.com" }
        { "domain" = "anotherorg.net" } }
   {}
   { "#comment" = "A sortlist now" }
   { "sortlist"
        { "ipaddr" = "130.155.160.0"
           { "netmask" = "255.255.240.0" } }
        { "ipaddr" = "130.155.0.0" } }
   {}
   { "options"
        { "ndots" = "3" }
        { "debug" }
        { "timeout" = "2" } }
   { "options"
        { "ip6-dotint"
             { "negate" } } }

test Resolv.ip6_dotint
   put "ip6-dotint"
   after set "/ip6-dotint/negate" "" = "no-ip6-dotint"


