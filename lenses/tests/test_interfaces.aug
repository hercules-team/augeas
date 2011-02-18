module Test_interfaces =

    let conf ="# This file describes the network interfaces available on your system
# and how to activate them. For more information, see interfaces(5).
# The loopback network interface

auto lo eth0 #foo
allow-hotplug eth1

iface lo inet \
 loopback

mapping eth0
	script /usr/local/sbin/map-scheme
map HOME eth0-home
     map \
 WORK eth0-work

iface eth0-home inet static

address 192.168.1.1
     netmask 255.255.255.0
     bridge_maxwait 0
#        up flush-mail
    down Mambo #5

  iface eth0-work inet dhcp

allow-auto eth1
iface eth1 inet dhcp

mapping eth1
	# I like mapping ...
        # ... and I like comments

	script\
 /usr/local/sbin/map-scheme
"

    test Interfaces.lns get conf =
        { "#comment" = "This file describes the network interfaces available on your system"}
        { "#comment" = "and how to activate them. For more information, see interfaces(5)." }
        { "#comment" = "The loopback network interface" }
        {}
        { "auto"
            { "1" = "lo" }
            { "2" = "eth0" }
            { "3" = "#foo" } }
        { "allow-hotplug" { "1" = "eth1" } }
        { }
        { "iface" = "lo"
            { "family" = "inet"}
            { "method" = "loopback"} {} }
        { "mapping" = "eth0"
            { "script" = "/usr/local/sbin/map-scheme"}
            { "map" = "HOME eth0-home"}
            { "map" = "WORK eth0-work"}
            {} }
        { "iface" = "eth0-home"
            { "family" = "inet"}
            { "method" = "static"}
            {}
            { "address" = "192.168.1.1" }
            { "netmask" = "255.255.255.0" }
            { "bridge_maxwait" = "0" }
            { "#comment" = "up flush-mail" }
            { "down" = "Mambo #5" }
            {} }
        { "iface" = "eth0-work"
            { "family" = "inet"}
            { "method" = "dhcp"}
            {} }
        { "auto"
            { "1" = "eth1" } }
        { "iface" = "eth1"
            { "family" = "inet"}
            { "method" = "dhcp"}
	    {} }
        { "mapping" = "eth1"
            { "#comment" = "I like mapping ..." }
            { "#comment" = "... and I like comments" }
            {}
            { "script" = "/usr/local/sbin/map-scheme"} }

test Interfaces.lns put "" after
	set "/iface[1]" "eth0";
	set "/iface[1]/family" "inet";
	set "/iface[1]/method" "dhcp"
= "iface eth0 inet dhcp\n"
