module Test_resolv =

   let conf = "# Sample resolv.conf
; With multiple comment styles
nameserver 192.168.0.3  # and EOL comments
nameserver ff02::1
domain mynet.com  # and EOL comments
search mynet.com anotherorg.net

# A sortlist now
sortlist 130.155.160.0/255.255.240.0 130.155.0.0

options ndots:3 debug timeout:2
options no-ip6-dotint single-request-reopen # and EOL comments

lookup file bind
family inet6 inet4
"

test Resolv.lns get conf =
   { "#comment" = "Sample resolv.conf" }
   { "#comment" = "With multiple comment styles" }
   { "nameserver" = "192.168.0.3"
        { "#comment" = "and EOL comments" } }
   { "nameserver" = "ff02::1" }
   { "domain" = "mynet.com"
        { "#comment" = "and EOL comments" } }
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
             { "negate" } }
	{ "single-request-reopen" }
        { "#comment" = "and EOL comments" } }
   {}
   { "lookup"
        { "file" }
        { "bind" } }
   { "family"
        { "inet6" }
        { "inet4" } }

test Resolv.ip6_dotint
   put "ip6-dotint"
   after set "/ip6-dotint/negate" "" = "no-ip6-dotint"


