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
"

(* Test: Protocols.lns *)
test Protocols.lns get conf =
  { "#comment" = "Internet (IP) protocols" }
  {  }
  { "ip" = "0"
    { "alias" = "IP" }
    { "#comment" = "internet protocol, pseudo protocol number" }
  }
  { "#comment" = "hopopt	0	HOPOPT		# IPv6 Hop-by-Hop Option [RFC1883]" }
  { "icmp" = "1"
    { "alias" = "ICMP" }
    { "#comment" = "internet control message protocol" }
  }
  { "igmp" = "2"
    { "alias" = "IGMP" }
    { "#comment" = "Internet Group Management" }
  }
