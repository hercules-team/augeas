(*
Module: Test_Protocols
  Provides unit tests and examples for the <Protocols> lens.
*)

module Test_Protocols =

(* Variable: conf *)
let conf = "# Internet (IP) protocols

ip	0	IP		# internet protocol, pseudo protocol number
#hopopt	0	HOPOPT		# IPv6 Hop-by-Hop Option [RFC1883]
icmp	1	ICMP		# internet control message protocol
igmp	2	IGMP		# Internet Group Management
tp++    39  TP++        # TP++ Transport Protocol
a/n 107 A/N     # Active Networks
"

(* Test: Protocols.lns *)
test Protocols.lns get conf =
  { "#comment" = "Internet (IP) protocols" }
  {  }
  { "1"
    { "protocol" = "ip" }
    { "number" = "0" }
    { "alias" = "IP" }
    { "#comment" = "internet protocol, pseudo protocol number" }
  }
  { "#comment" = "hopopt	0	HOPOPT		# IPv6 Hop-by-Hop Option [RFC1883]" }
  { "2"
    { "protocol" = "icmp" }
    { "number" = "1" }
    { "alias" = "ICMP" }
    { "#comment" = "internet control message protocol" }
  }
  { "3"
    { "protocol" = "igmp" }
    { "number" = "2" }
    { "alias" = "IGMP" }
    { "#comment" = "Internet Group Management" }
  }
  { "4"
    { "protocol" = "tp++" }
    { "number" = "39" }
    { "alias" = "TP++" }
    { "#comment" = "TP++ Transport Protocol" }
  }
  { "5"
    { "protocol" = "a/n" }
    { "number" = "107" }
    { "alias" = "A/N" }
    { "#comment" = "Active Networks" }
  }
