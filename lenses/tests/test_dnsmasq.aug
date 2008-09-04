module Test_dnsmasq =

let conf = "# Configuration file for dnsmasq.
#
#bogus-priv

conf-dir=/etc/dnsmasq.d
selfmx
"

test Dnsmasq.lns get conf =
  { "#comment" = "Configuration file for dnsmasq." }
  {}
  { "#comment" = "bogus-priv" }
  {}
  { "conf-dir" = "/etc/dnsmasq.d" }
  { "selfmx" }
