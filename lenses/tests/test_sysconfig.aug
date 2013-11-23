(* Test for sysconfig lens *)
module Test_sysconfig =

  let lns = Sysconfig.lns

  let eth_static = "# Intel Corporation PRO/100 VE Network Connection
DEVICE=eth0
BOOTPROTO=static
BROADCAST=172.31.0.255
HWADDR=ab:cd:ef:12:34:56
export IPADDR=172.31.0.31 # this is our IP
#DHCP_HOSTNAME=host.example.com
NETMASK=255.255.255.0
NETWORK=172.31.0.0
unset ONBOOT    #   We do not want this var
"
  let empty_val = "EMPTY=\nDEVICE=eth0\n"

  let key_brack = "SOME_KEY[1]=\nDEVICE=eth0\n"

  test lns get eth_static =
    { "#comment" = "Intel Corporation PRO/100 VE Network Connection" }
    { "DEVICE" = "eth0" }
    { "BOOTPROTO" = "static" }
    { "BROADCAST" = "172.31.0.255" }
    { "HWADDR" = "ab:cd:ef:12:34:56" }
    { "IPADDR" = "172.31.0.31"
        { "export" }
        { "#comment" = "this is our IP" } }
    { "#comment" = "DHCP_HOSTNAME=host.example.com" }
    { "NETMASK" = "255.255.255.0" }
    { "NETWORK" = "172.31.0.0" }
    { "@unset"
      { "1" = "ONBOOT" }
      { "#comment" = "We do not want this var" } }

  test lns put eth_static after
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
unset ONBOOT    #   We do not want this var
"
  test lns get empty_val =
    { "EMPTY" = "" } { "DEVICE" = "eth0" }

  test lns get key_brack =
    { "SOME_KEY[1]" = "" } { "DEVICE" = "eth0" }

  test lns get "smartd_opts=\"-q never\"\n" =
    { "smartd_opts" = "-q never" }

  test lns get "var=val  \n" = { "var" = "val" }

  test lns get ". /etc/java/java.conf\n" =
    { ".source" = "/etc/java/java.conf" }

  (* Quoted strings and other oddities *)
  test lns get "var=\"foo 'bar'\"\n" =
    { "var" = "foo 'bar'" }

  test lns get "var=\"eth0\"\n" =
    { "var" = "eth0" }

  test lns get "var='eth0'\n" =
    { "var" = "eth0" }

  test lns get "var='Some \"funny\" value'\n" =
    { "var" = "Some \"funny\" value" }

  test lns get "var=\"\\\"\"\n" =
    { "var" = "\\\"" }

  test lns get "var=\\\"\n" =
    { "var" = "\\\"" }

  test lns get "var=ab#c\n" =
    { "var" = "ab#c" }

  test lns get "var='ab#c'\n" =
    { "var" = "ab#c" }

  test lns get "var=\"ab#c\"\n" =
    { "var" = "ab#c" }

  test lns get "var=\"ab#c\"\n" =
    { "var" = "ab#c" }

  (* We don't handle backticks *)
  test lns get
      "var=`grep nameserver /etc/resolv.conf | head -1`\n" = *

  test lns get "var=ab #c\n" =
    { "var" = "ab"
        { "#comment" = "c" } }

  test lns put "var=ab #c\n"
    after rm "/var/#comment" = "var=ab\n"

  test lns put "var=ab\n"
    after set "/var/#comment" "this is a var" =
       "var=ab # this is a var\n"

  (* Test semicolons *)
  test lns get "VAR1=\"this;is;a;test\"\nVAR2=this;\n" =
  { "VAR1" = "this;is;a;test" }
  { "VAR2" = "this" }

  (* BZ 761246 *)
  test lns get "DEVICE=\"eth0\";\n" =
    { "DEVICE" = "eth0" }

  test lns put "DEVICE=\"eth0\";\n" after
    set "/DEVICE" "em1" = "DEVICE=\"em1\";\n"

  test lns get "DEVICE=\"eth0\"; # remark\n" =
    { "DEVICE" = "eth0" }
    { "#comment" = "remark" }

  (* Bug 109: allow a bare export *)
  test lns get "export FOO\n" =
  { "@export"
    { "1" = "FOO" } }

  (* Check we put quotes in when changes require them *)
  test lns put "var=\"v\"\n" after rm "/foo" =
    "var=\"v\"\n"

  test lns put "var=v\n" after set "/var" "v w"=
    "var=\"v w\"\n"

  test lns put "var='v'\n" after set "/var" "v w"=
    "var='v w'\n"

  test lns put "var=v\n" after set "/var" "v'w"=
    "var=\"v'w\"\n"

  test lns put "var=v\n" after set "/var" "v\"w"=
    "var='v\"w'\n"

  (* RHBZ#1043636: empty comment lines after comments *)
  test lns get "#MOUNTD_NFS_V3\n#\n" =
    { "#comment" = "MOUNTD_NFS_V3" }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
