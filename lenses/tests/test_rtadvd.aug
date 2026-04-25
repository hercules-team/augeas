module Test_rtadvd =

(* Example from rtadvd.conf(5) *)
let rtadvd_conf = "default:\\
        :chlim#64:raflags#0:rltime#1800:rtime#0:retrans#0:\\
        :pinfoflags=\"la\":vltime#2592000:pltime#604800:mtu#0:
ef0:\\
        :addr=\"2001:db8:ffff:1000::\":prefixlen#64:tc=default:
"

test Rtadvd.lns get rtadvd_conf =
  { "record"
    { "name" = "default" }
    { "capability" = "chlim#64" }
    { "capability" = "raflags#0" }
    { "capability" = "rltime#1800" }
    { "capability" = "rtime#0" }
    { "capability" = "retrans#0" }
    { "capability" = "pinfoflags=\"la\"" }
    { "capability" = "vltime#2592000" }
    { "capability" = "pltime#604800" }
    { "capability" = "mtu#0" }
  }
  { "record"
    { "name" = "ef0" }
    { "capability" = "addr=\"2001:db8:ffff:1000::\"" }
    { "capability" = "prefixlen#64" }
    { "capability" = "tc=default" }
  }

