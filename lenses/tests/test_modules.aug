module Test_Modules =

let conf = "# /etc/modules: kernel modules to load at boot time.

lp
rtc
"

test Modules.lns get conf =
  { "#comment" = "/etc/modules: kernel modules to load at boot time." }
  {  }
  { "lp" }
  { "rtc" }
