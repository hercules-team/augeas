module Test_dhclient =

    let conf ="  # Sample dhclient.conf
    # Protocol timing
timeout 3; # Expect a fast server
retry
      10;
# Lease requirements and requests
request
	subnet-mask,
  broadcast-address,
	ntp-servers;
# Dynamic DNS
send
	fqdn.fqdn
	  \"grosse.fugue.com.\";

option rfc3442-classless-static-routes code 121 = array of unsigned integer 8;

interface ep0 {
   script /sbin/dhclient-script;
   send dhcp-client-identifier 1:0:a0:24:ab:fb:9c;
   send dhcp-lease-time 3600;
   request subnet-mask, broadcast-address, time-offset, routers,
          domain-name, domain-name-servers, host-name;
   media media10baseT/UTP, \"media10base2/BNC\";
}

alias {
  interface \"ep0\";
  fixed-address 192.5.5.213;
  option subnet-mask 255.255.255.255;
}

lease {
  interface \"eth0\";
  fixed-address 192.33.137.200;
  medium \"link0 link1\";
  vendor option space \"name\";
  option host-name \"andare.swiftmedia.com\";
  option subnet-mask 255.255.255.0;
  option broadcast-address 192.33.137.255;
  option routers 192.33.137.250;
  option domain-name-servers 127.0.0.1;
  renew 2 2000/1/12 00:00:01;
  rebind 2 2000/1/12 00:00:01;
  expire 2 2000/1/12 00:00:01;
}
"

    test Dhclient.lns get conf =
        { "#comment" = "Sample dhclient.conf" }
        { "#comment" = "Protocol timing" }
        { "timeout" = "3"
           { "#comment" = "Expect a fast server" } }
        { "retry" = "10" }
        { "#comment" = "Lease requirements and requests" }
        { "request"
           { "1" = "subnet-mask" }
           { "2" = "broadcast-address" }
           { "3" = "ntp-servers" } }
        { "#comment" = "Dynamic DNS" }
        { "send"
           { "fqdn.fqdn" = "\"grosse.fugue.com.\"" } }
        {}
        { "option"
           { "rfc3442-classless-static-routes"
               { "code"  = "121" }
               { "value" = "array of unsigned integer 8" } } }
        {}
        { "interface" = "ep0"
           { "script" = "/sbin/dhclient-script" }
           { "send"
               { "dhcp-client-identifier" = "1:0:a0:24:ab:fb:9c" } }
           { "send"
               { "dhcp-lease-time" = "3600" } }
           { "request"
               { "1" = "subnet-mask" }
               { "2" = "broadcast-address" }
               { "3" = "time-offset" }
               { "4" = "routers" }
               { "5" = "domain-name" }
               { "6" = "domain-name-servers" }
               { "7" = "host-name" } }
           { "media"
               { "1" = "media10baseT/UTP" }
               { "2" = "\"media10base2/BNC\"" } } }
        {}
        { "alias"
           { "interface" = "\"ep0\"" }
           { "fixed-address" = "192.5.5.213" }
           { "option"
               { "subnet-mask" = "255.255.255.255" } } }
        {}
        { "lease"
           { "interface" = "\"eth0\"" }
           { "fixed-address" = "192.33.137.200" }
           { "medium" = "\"link0 link1\"" }
           { "vendor option space" = "\"name\"" }
           { "option"
               { "host-name" = "\"andare.swiftmedia.com\"" } }
           { "option"
               { "subnet-mask" = "255.255.255.0" } }
           { "option"
               { "broadcast-address" = "192.33.137.255" } }
           { "option"
               { "routers" = "192.33.137.250" } }
           { "option"
               { "domain-name-servers" = "127.0.0.1" } }
           { "renew"
               { "weekday" = "2" }
               { "year"    = "2000" }
               { "month"   = "1" }
               { "day"     = "12" }
               { "hour"    = "00" }
               { "minute"  = "00" }
               { "second"  = "01" } }
           { "rebind"
               { "weekday" = "2" }
               { "year"    = "2000" }
               { "month"   = "1" }
               { "day"     = "12" }
               { "hour"    = "00" }
               { "minute"  = "00" }
               { "second"  = "01" } }
           { "expire"
               { "weekday" = "2" }
               { "year"    = "2000" }
               { "month"   = "1" }
               { "day"     = "12" }
               { "hour"    = "00" }
               { "minute"  = "00" }
               { "second"  = "01" } } }
