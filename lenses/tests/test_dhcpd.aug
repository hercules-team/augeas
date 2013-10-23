module Test_dhcpd =

let lns = Dhcpd.lns

let conf = "#
# Sample configuration file for ISC dhcpd for Debian
#
# Attention: If /etc/ltsp/dhcpd.conf exists, that will be used as
# configuration file instead of this file.
#
# $Id: dhcpd.conf,v 1.1.1.1 2002/05/21 00:07:44 peloy Exp $
#

# The ddns-updates-style parameter controls whether or not the server will
# attempt to do a DNS update when a lease is confirmed. We default to the
# behavior of the version 2 packages ('none', since DHCP v2 didn't
# have support for DDNS.)
ddns-update-style none;

# option definitions common to all supported networks...
option domain-name \"example.org\";
option domain-name-servers ns1.example.org, ns2.example.org;

default-lease-time 600;
max-lease-time 7200;

# If this DHCP server is the official DHCP server for the local
# network, the authoritative directive should be uncommented.
authoritative;

allow booting;
allow bootp;

# Use this to send dhcp log messages to a different log file (you also
# have to hack syslog.conf to complete the redirection).
log-facility local7;

# No service will be given on this subnet, but declaring it helps the
# DHCP server to understand the network topology.

subnet 10.152.187.0 netmask 255.255.255.0 {
}

# This is a very basic subnet declaration.

subnet 10.254.239.0 netmask 255.255.255.224 {
  range 10.254.239.10 10.254.239.20;
  option routers rtr-239-0-1.example.org, rtr-239-0-2.example.org;
}

# This declaration allows BOOTP clients to get dynamic addresses,
# which we don't really recommend.

subnet 10.254.239.32 netmask 255.255.255.224 {
  range dynamic-bootp 10.254.239.40 10.254.239.60;
  option broadcast-address 10.254.239.31;
  option routers rtr-239-32-1.example.org;
}

# A slightly different configuration for an internal subnet.
subnet 10.5.5.0 netmask 255.255.255.224 {
  range 10.5.5.26 10.5.5.30;
  option domain-name-servers ns1.internal.example.org;
  option domain-name \"internal.example.org\";
  option routers 10.5.5.1;
  option broadcast-address 10.5.5.31;
  default-lease-time 600;
  max-lease-time 7200;
}

# Hosts which require special configuration options can be listed in
# host statements.   If no address is specified, the address will be
# allocated dynamically (if possible), but the host-specific information
# will still come from the host declaration.

host passacaglia {
  hardware ethernet 0:0:c0:5d:bd:95;
  filename \"vmunix.passacaglia\";
  server-name \"toccata.fugue.com\";
}

# Fixed IP addresses can also be specified for hosts.   These addresses
# should not also be listed as being available for dynamic assignment.
# Hosts for which fixed IP addresses have been specified can boot using
# BOOTP or DHCP.   Hosts for which no fixed address is specified can only
# be booted with DHCP, unless there is an address range on the subnet
# to which a BOOTP client is connected which has the dynamic-bootp flag
# set.
host fantasia {
  hardware ethernet 08:00:07:26:c0:a5;
  fixed-address fantasia.fugue.com;
}

# You can declare a class of clients and then do address allocation
# based on that.   The example below shows a case where all clients
# in a certain class get addresses on the 10.17.224/24 subnet, and all
# other clients get addresses on the 10.0.29/24 subnet.

#class \"foo\" {
#  match if substring (option vendor-class-identifier, 0, 4) = \"SUNW\";
#}

shared-network 224-29 {
  subnet 10.17.224.0 netmask 255.255.255.0 {
    option routers rtr-224.example.org;
  }
  subnet 10.0.29.0 netmask 255.255.255.0 {
    option routers rtr-29.example.org;
  }
  pool {
    allow members of \"foo\";
    range 10.17.224.10 10.17.224.250;
  }
  pool {
    deny members of \"foo\";
    range 10.0.29.10 10.0.29.230;
  }
}
"

test lns get "authoritative;" = { "authoritative" }
test lns get "ddns-update-style none;" = { "ddns-update-style" = "none" }
test lns get "option domain-name \"example.org\";" =
  { "option"
    { "domain-name"
      { "arg" = "example.org" }
    }
  }

test lns get "option domain-name-servers ns1.example.org, ns2.example.org;" =
  { "option"
    { "domain-name-servers"
      { "arg" = "ns1.example.org" }
      { "arg" = "ns2.example.org" }
    }
  }

test lns get "default-lease-time 600;" = { "default-lease-time" = "600" }
test lns get "range 10.254.239.60;" =
{ "range"
    { "to" = "10.254.239.60" }
  }

test lns get "range dynamic-bootp 10.254.239.60;" =
  { "range"
    { "flag" = "dynamic-bootp" }
    { "to" = "10.254.239.60" }
  }

test lns get "range dynamic-bootp 10.254.239.40 10.254.239.60;" =
  { "range"
    { "flag" = "dynamic-bootp" }
    { "from" = "10.254.239.40" }
    { "to" = "10.254.239.60" }
  }

test lns get "subnet 10.152.187.0 netmask 255.255.255.0 {}\n" =
  { "subnet"
    { "network" = "10.152.187.0" }
    { "netmask" = "255.255.255.0" }
  }

test lns get " pool {
    pool {

    }
}
" =
  { "pool"
    { "pool" }
  }

test lns get "group { host some-host {hardware ethernet 00:00:aa:bb:cc:dd;
fixed-address 10.1.1.1;}}" =
  { "group"
    { "host" = "some-host"
      { "hardware"
        { "type" = "ethernet" }
        { "address" = "00:00:aa:bb:cc:dd" }
      }
      { "fixed-address" = "10.1.1.1" }
    }
  }

test Dhcpd.stmt_secu get "allow members of \"foo\";" =  { "allow-members-of" = "foo" }
test Dhcpd.stmt_secu get "allow booting;" =  { "allow" = "booting" }
test Dhcpd.stmt_secu get "allow bootp;" =  { "allow" = "bootp" }
test Dhcpd.stmt_option get "option voip-boot-server code 66 = string;" =
  { "rfc-code"
    { "label" = "voip-boot-server" }
    { "code" = "66" }
    { "type" = "string" }
  }

test Dhcpd.lns get "authoritative;
log-facility local7;
ddns-update-style none;
default-lease-time 21600;
max-lease-time 43200;

# Additional options for VOIP
option voip-boot-server code 66 = string;
option voip-vlan-id code 128 = string;
" =
  { "authoritative" }
  { "log-facility" = "local7" }
  { "ddns-update-style" = "none" }
  { "default-lease-time" = "21600" }
  { "max-lease-time" = "43200"
    { "#comment" = "Additional options for VOIP" }
  }
  { "rfc-code"
    { "label" = "voip-boot-server" }
    { "code" = "66" }
    { "type" = "string" }
  }
  { "rfc-code"
    { "label" = "voip-vlan-id" }
    { "code" = "128" }
    { "type" = "string" }
  }


test Dhcpd.lns get "
option domain-name-servers 10.1.1.1, 10.11.2.1, 10.1.3.1;
next-server 10.1.1.1;

failover peer \"redondance01\" {
         primary;
         address 10.1.1.1;
         port 647;
         peer address 10.1.1.1;
         peer port 647;
         max-response-delay 20;
         max-unacked-updates 10;
         mclt 3600;         #comment.
         split 128;         #comment.
         load balance max seconds 3;
       }
" =
  {  }
  { "option"
    { "domain-name-servers"
      { "arg" = "10.1.1.1" }
      { "arg" = "10.11.2.1" }
      { "arg" = "10.1.3.1" }
    }
  }
  { "next-server" = "10.1.1.1" }
  { "failover peer" = "\"redondance01\""
    { "primary" }
    { "address" = "10.1.1.1" }
    { "port" = "647" }
    { "peer address" = "10.1.1.1" }
    { "peer port" = "647" }
    { "max-response-delay" = "20" }
    { "max-unacked-updates" = "10" }
    { "mclt" = "3600"
      { "#comment" = "comment." }
    }
    { "split" = "128"
      { "#comment" = "comment." }
    }
    { "load balance max seconds" = "3" }
  }

test Dhcpd.lns get "
option CallManager code 150 = ip-address;
option slp-directory-agent true 10.1.1.1, 10.2.2.2;
option slp-service-scope true \"SLP-GLOBAL\";
option nds-context \"EXAMPLE\";
option nds-tree-name \"EXAMPLE\";
" =
  {  }
  { "rfc-code"
    { "label" = "CallManager" }
    { "code" = "150" }
    { "type" = "ip-address" }
  }
  { "option"
    { "slp-directory-agent" = "true"
      { "arg" = "10.1.1.1" }
      { "arg" = "10.2.2.2" }
    }
  }
  { "option"
    { "slp-service-scope" = "true"
      { "arg" = "SLP-GLOBAL" }
    }
  }
  { "option"
    { "nds-context"
      { "arg" = "EXAMPLE" }
    }
  }
  { "option"
    { "nds-tree-name"
      { "arg" = "EXAMPLE" }
    }
  }


test Dhcpd.lns get "option voip-vlan-id \"VLAN=1234;\";" =
  { "option"
    { "voip-vlan-id"
      { "arg" = "VLAN=1234;" }
    }
  }

test Dhcpd.lns get "option domain-name \"x.example.com y.example.com z.example.com\";" =
  { "option"
    { "domain-name"
      { "arg" = "x.example.com y.example.com z.example.com" }
    }
  }

test Dhcpd.lns get "include \"/etc/dhcpd.master\";" =
  { "include" = "/etc/dhcpd.master" }

test Dhcpd.lns put "\n" after set "/include" "/etc/dhcpd.master" =
  "\ninclude \"/etc/dhcpd.master\";\n"

test Dhcpd.fct_args get "(option dhcp-client-identifier, 1, 3)" =
  { "args"
    { "arg" = "option dhcp-client-identifier" }
    { "arg" = "1" }
    { "arg" = "3" }
  }

test Dhcpd.stmt_match get "match if substring (option dhcp-client-identifier, 1, 3) = \"RAS\";" =
  { "match"
    { "function" = "substring"
      { "args"
        { "arg" = "option dhcp-client-identifier" }
        { "arg" = "1" }
        { "arg" = "3" }
      }
    }
    { "value" = "RAS" }
  }

test Dhcpd.lns get "match pick-first-value (option dhcp-client-identifier, hardware);" =
  { "match"
    { "function" = "pick-first-value"
      { "args"
        { "arg" = "option dhcp-client-identifier" }
        { "arg" = "hardware"  }
      }
    }
  }

test Dhcpd.fct_args get "(16, 32, \"\", substring(hardware, 0, 4))" =
  { "args"
    { "arg" = "16" }
    { "arg" = "32" }
    { "arg" = "\"\"" }
    { "arg" = "substring(hardware, 0, 4)" }
  }

test Dhcpd.stmt_match get "match if binary-to-ascii(16, 32, \"\", substring(hardware, 0, 4)) = \"1525400\";" =
  { "match"
    { "function" = "binary-to-ascii"
      { "args"
        { "arg" = "16" }
        { "arg" = "32" }
        { "arg" = "\"\"" }
        { "arg" = "substring(hardware, 0, 4)" }
      }
    }
    { "value" = "1525400" }
  }

test Dhcpd.lns get "subclass \"allocation-class-1\" 1:8:0:2b:4c:39:ad;" =
  { "subclass"
    { "name" = "allocation-class-1" }
    { "value" = "1:8:0:2b:4c:39:ad" }
  }

(* overall test *)
test Dhcpd.lns put conf after rm "/x" = conf

(* bug #293: primary should support argument *)
let input293 = "zone EXAMPLE.ORG. {
  primary 127.0.0.1;
}"

test Dhcpd.lns get input293 = 
  { "zone" = "EXAMPLE.ORG."
    { "primary" = "127.0.0.1" }
  }

(* bug #311: filename should be quoted *)
let input311 = "subnet 172.16.0.0 netmask 255.255.255.0 {
filename \"pxelinux.0\";
}"

test Dhcpd.lns put "subnet 172.16.0.0 netmask 255.255.255.0 {
}" after
  set "subnet/filename" "pxelinux.0" = input311

(* GH issue #34: support conditional structures *)
let gh34_empty = "if exists dhcp-parameter-request-list {
}\n"

test Dhcpd.lns get gh34_empty =
  { "@if" = "exists dhcp-parameter-request-list" }

let gh34_empty_multi = "subnet 192.168.100.0 netmask 255.255.255.0 {
 if true {
 } elsif false {
 } else {
 }
}\n"

test Dhcpd.lns get gh34_empty_multi =
  { "subnet"
    { "network" = "192.168.100.0" }
    { "netmask" = "255.255.255.0" }
    { "@if" = "true"
      { "@elsif" = "false" }
      { "@else" } }
  }

let gh34_simple = "if exists dhcp-parameter-request-list {
  default-lease-time 600;
  } else {
default-lease-time 200;
}\n"

test Dhcpd.lns get gh34_simple =
  { "@if" = "exists dhcp-parameter-request-list"
    { "default-lease-time" = "600" }
    { "@else"
      { "default-lease-time" = "200" } } }

