module Test_IPRoute2 =

let conf = "
# /etc/iproute2/rt_tables
#
# reserved values
#
255	local
254	main
253	default
0	unspec
#
# local
#
#1	inr.ruhep
200	h3g0
201	adsl1
202	adsl2
203	adsl3
204	adsl4
205	wifi0
#
# From rt_dsfield
#
0x00  default
0x80  flash-override

# From rt_protos
#
254 gated/aggr
253 gated/bgp
"

test IPRoute2.lns get conf =
  { }
  { "#comment" = "/etc/iproute2/rt_tables" }
  { }
  { "#comment" = "reserved values" }
  { }
  { "255" = "local" }
  { "254" = "main" }
  { "253" = "default" }
  { "0" = "unspec" }
  { }
  { "#comment" = "local" }
  { }
  { "#comment" = "1	inr.ruhep" }
  { "200" = "h3g0" }
  { "201" = "adsl1" }
  { "202" = "adsl2" }
  { "203" = "adsl3" }
  { "204" = "adsl4" }
  { "205" = "wifi0" }
  { }
  { "#comment" = "From rt_dsfield" }
  { }
  { "0x00" = "default" }
  { "0x80" = "flash-override" }
  { }
  { "#comment" = "From rt_protos" }
  { }
  { "254" = "gated/aggr" }
  { "253" = "gated/bgp" }
