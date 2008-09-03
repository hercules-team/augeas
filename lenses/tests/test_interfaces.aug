module Test_interfaces = 

    let conf ="# This file describes the network interfaces available on your system
# and how to activate them. For more information, see interfaces(5).
# The loopback network interface

auto lo eth0 # Take me to the net
allow-hotplug eth1

iface lo inet \
 loopback

mapping eth0 # Home sweet home
	script /usr/local/sbin/map-scheme
map HOME eth0-home
     map \
 WORK eth0-work

iface eth0-home inet static

address 192.168.1.1
     netmask 255.255.255.0
#        up flush-mail

iface eth0-work inet dhcp

allow-auto eth1
iface eth1 inet dhcp # This is easy

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
	    { "#comment" = "Take me to the net" } }
        { "allow-hotplug" = "eth1" }
        { "iface" = "lo"
            { "family" = "inet"}
            { "method" = "loopback"} {} {} }
        { "mapping" = "eth0"
            { "#comment" = "Home sweet home" }
            { "script" = "/usr/local/sbin/map-scheme"}
            { "map" = "HOME eth0-home"}
            { "map" = "WORK eth0-work"} 
            {} }
        { "iface" = "eth0-home"
            { "family" = "inet"}
            { "method" = "static"}
            {} {}
            { "address" = "192.168.1.1" }
            { "netmask" = "255.255.255.0" }
            { "#comment" = "up flush-mail" }
            {} }
        { "iface" = "eth0-work"
            { "family" = "inet"}
            { "method" = "dhcp"}
            {} {} }
        { "auto"
            { "1" = "eth1" } }
        { "iface" = "eth1"
            { "family" = "inet"}
            { "method" = "dhcp"}
            { "#comment" = "This is easy" }
	    {} }
        { "mapping" = "eth1"
            {}
            { "#comment" = "I like mapping ..." }
            { "#comment" = "... and I like comments" }
            {}
            { "script" = "/usr/local/sbin/map-scheme"} }
