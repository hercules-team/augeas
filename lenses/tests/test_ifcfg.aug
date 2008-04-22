(* Test for Ifcfg module *)
module Test_ifcfg =
  let eth_static = "# Intel Corporation PRO/100 VE Network Connection
DEVICE=eth0
BOOTPROTO=static
BROADCAST=172.31.0.255
HWADDR=ab:cd:ef:12:34:56
IPADDR=172.31.0.31
#DHCP_HOSTNAME=host.example.com
NETMASK=255.255.255.0
NETWORK=172.31.0.0
ONBOOT=yes
"
  let empty_val = "EMPTY=\nDEVICE=eth0\n"

  let key_brack = "SOME_KEY[1]=\nDEVICE=eth0\n"

  test Ifcfg.lns get eth_static =
    { }
    { "DEVICE" = "eth0" }
    { "BOOTPROTO" = "static" }
    { "BROADCAST" = "172.31.0.255" }
    { "HWADDR" = "ab:cd:ef:12:34:56" }
    { "IPADDR" = "172.31.0.31" }
    { }
    { "NETMASK" = "255.255.255.0" }
    { "NETWORK" = "172.31.0.0" }
    { "ONBOOT" = "yes" }

  test Ifcfg.lns put eth_static after
      set "BOOTPROTO" "dhcp" ;
      rm "IPADDR" ;
      rm "BROADCAST" ;
      rm "NETMASK" ;
      rm "NETWORK"
  = "# Intel Corporation PRO/100 VE Network Connection
DEVICE=eth0
BOOTPROTO=dhcp
HWADDR=ab:cd:ef:12:34:56
#DHCP_HOSTNAME=host.example.com
ONBOOT=yes
"
  test Ifcfg.lns get empty_val = 
    { "EMPTY" = "" } { "DEVICE" = "eth0" }

  test Ifcfg.lns get key_brack = 
    { "SOME_KEY[1]" = "" } { "DEVICE" = "eth0" }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
