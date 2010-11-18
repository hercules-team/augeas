(* Test for shell lens *)
module Test_shellvars =

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

  test Shellvars.lns get eth_static =
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
    { "@unset"   = "ONBOOT"
        { "#comment" = "We do not want this var" } }

  test Shellvars.lns put eth_static after
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
  test Shellvars.lns get empty_val =
    { "EMPTY" = "" } { "DEVICE" = "eth0" }

  test Shellvars.lns get key_brack =
    { "SOME_KEY[1]" = "" } { "DEVICE" = "eth0" }

  test Shellvars.lns get "smartd_opts=\"-q never\"\n" =
    { "smartd_opts" = "\"-q never\"" }

  test Shellvars.lns get "var=val  \n" = { "var" = "val" }

  test Shellvars.lns get ". /etc/java/java.conf\n" =
    { ".source" = "/etc/java/java.conf" }

  (* Quoted strings and other oddities *)
  test Shellvars.lns get "var=\"foo 'bar'\"\n" =
    { "var" = "\"foo 'bar'\"" }

  test Shellvars.lns get "var='Some \"funny\" value'\n" =
    { "var" = "'Some \"funny\" value'" }

  test Shellvars.lns get "var=\"\\\"\"\n" =
    { "var" = "\"\\\"\"" }

  test Shellvars.lns get "var=\\\"\n" =
    { "var" = "\\\"" }

  test Shellvars.lns get "var=ab#c\n" =
    { "var" = "ab"
        { "#comment" = "c" } }

  test Shellvars.lns get "var='ab#c'\n" =
    { "var" = "'ab#c'" }

  test Shellvars.lns get "var=\"ab#c\"\n" =
    { "var" = "\"ab#c\"" }

  (* For some reason, `` conflicts with comment_eol *)
  test Shellvars.lns get "var=`ab#c`\n" =
    { "var" = "`ab"
       { "#comment" = "c`" } }

  test Shellvars.lns get "var=`grep nameserver /etc/resolv.conf | head -1`\n" =
    { "var" = "`grep nameserver /etc/resolv.conf | head -1`" }

  test Shellvars.lns put "var=ab #c\n"
    after rm "/var/#comment" = "var=ab\n"

  test Shellvars.lns put "var=ab\n"
    after set "/var/#comment" "this is a var" =
       "var=ab # this is a var\n"

  (* Handling of arrays *)
  test Shellvars.lns get "var=(val1 \"val\\\"2\\\"\" val3)\n" =
    { "var"
        { "1" = "val1" }
        { "2" = "\"val\\\"2\\\"\"" }
        { "3" = "val3" } }

  test Shellvars.lns get "var=()\n" = { "var" = "()" }

  test Shellvars.lns put "var=()\n" after
      set "var" "value"
  = "var=value\n"

  test Shellvars.lns put "var=(v1 v2)\n" after
      rm "var/*" ;
      set "var" "value"
  = "var=value\n"

  test Shellvars.lns put "var=(v1 v2)\n" after
    set "var/3" "v3"
  = "var=(v1 v2 v3)\n"

  test Shellvars.lns get "var=(v1 v2   \n    \t v3)\n" =
  { "var"
    { "1" = "v1" }
    { "2" = "v2" }
    { "3" = "v3" } }

  (* Allow spaces after/before opening/closing parens for array *)
  test Shellvars.lns get "config_eth1=( \"10.128.0.48/24\" )\n" =
  { "config_eth1"  { "1" = "\"10.128.0.48/24\"" } }

  (* Bug 109: allow a bare export *)
  test Shellvars.lns get "export FOO\n" =
  { "@export" = "FOO" }

  (* Bug 73: allow ulimit builtin *)
  test Shellvars.lns get "ulimit -c unlimited\n" =
  { "@builtin" = "ulimit" { "args" = "-c unlimited" } }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
