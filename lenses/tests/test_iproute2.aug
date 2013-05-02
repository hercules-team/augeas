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
"

test IPRoute2.lns get conf =
  { }
  { "#comment" = "/etc/iproute2/rt_tables" }
  { }
  { "#comment" = "reserved values" }
  { }
  { "local" = "255" }
  { "main" = "254" }
  { "default" = "253" }
  { "unspec" = "0" }
  { }
  { "#comment" = "local" }
  { }
  { "#comment" = "1	inr.ruhep" }
  { "h3g0" = "200" }
  { "adsl1" = "201" }
  { "adsl2" = "202" }
  { "adsl3" = "203" }
  { "adsl4" = "204" }
  { "wifi0" = "205" }
