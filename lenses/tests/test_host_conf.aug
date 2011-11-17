module Test_Host_Conf =

let conf = "
# /etc/host.conf
# We have named running, but no NIS (yet)
order   bind,hosts
# Allow multiple addrs
multi   on
# Guard against spoof attempts
nospoof on
# Trim local domain (not really necessary).
trim    vbrew.com.:fedora.org.
trim  augeas.net.,ubuntu.com.
"

test Host_Conf.lns get conf =
  {  }
  { "#comment" = "/etc/host.conf" }
  { "#comment" = "We have named running, but no NIS (yet)" }
  { "order"
    { "1" = "bind" }
    { "2" = "hosts" }
  }
  { "#comment" = "Allow multiple addrs" }
  { "multi" = "on" }
  { "#comment" = "Guard against spoof attempts" }
  { "nospoof" = "on" }
  { "#comment" = "Trim local domain (not really necessary)." }
  { "trim"
    { "1" = "vbrew.com." }
    { "2" = "fedora.org." }
  }
  { "trim"
    { "3" = "augeas.net." }
    { "4" = "ubuntu.com." }
  }
