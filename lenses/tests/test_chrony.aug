(*
Module: Test_Chrony
  Provides unit tests and examples for the <Chrony> lens.
*)

module Test_Chrony =

  let exampleconf = "# Comment
#Comment
! Comment
!Comment
; Comment
;Comment
% Comment
%Comment

server ntp1.example.com
server ntp2.example.com iburst
server ntp3.example.com presend 2
server ntp4.example.com offline polltarget 4
server ntp5.example.com maxdelay 2 offline
server ntp6.example.com maxdelay 2 iburst presend 2
server ntp7.example.com iburst presend 2 offline
peer ntpc1.example.com
stratumweight 0
driftfile /var/lib/chrony/drift
rtcsync
makestep 10 3
bindcmdaddress 127.0.0.1
bindcmdaddress ::1
local stratum 10
keyfile /etc/chrony.keys
commandkey 1
generatecommandkey
noclientlog
logchange 0.5
logdir /var/log/chrony
log rtc measurements
leapsectz right/UTC
broadcast 10 192.168.1.255
broadcast 10 192.168.100.255 123
fallbackdrift 16 19
mailonchange root@localhost 0.5
maxchange 1000 1 2
initstepslew 30 foo.bar.com
initstepslew 30 foo.bar.com baz.quz.com
"

  test Chrony.lns get exampleconf =
    { "#comment" = "Comment" }
  { "#comment" = "Comment" }
  { "#comment" = "Comment" }
  { "#comment" = "Comment" }
  { "#comment" = "Comment" }
  { "#comment" = "Comment" }
  { "#comment" = "Comment" }
  { "#comment" = "Comment" }
  {  }
  { "server" = "ntp1.example.com" }
  { "server" = "ntp2.example.com"
    { "iburst" }
  }
  { "server" = "ntp3.example.com"
    { "presend" = "2" }
  }
  { "server" = "ntp4.example.com"
    { "offline" }
    { "polltarget" = "4" }
  }
  { "server" = "ntp5.example.com"
    { "maxdelay" = "2" }
    { "offline" }
  }
  { "server" = "ntp6.example.com"
    { "maxdelay" = "2" }
    { "iburst" }
    { "presend" = "2" }
  }
  { "server" = "ntp7.example.com"
    { "iburst" }
    { "presend" = "2" }
    { "offline" }
  }
  { "peer" = "ntpc1.example.com" }
  { "stratumweight" = "0" }
  { "driftfile" = "/var/lib/chrony/drift" }
  { "rtcsync" }
  { "makestep"
    { "threshold" = "10" }
    { "limit" = "3" }
  }
  { "bindcmdaddress" = "127.0.0.1" }
  { "bindcmdaddress" = "::1" }
  { "local"
    { "stratum" = "10" }
  }
  { "keyfile" = "/etc/chrony.keys" }
  { "commandkey" = "1" }
  { "generatecommandkey" }
  { "noclientlog" }
  { "logchange" = "0.5" }
  { "logdir" = "/var/log/chrony" }
  { "log"
    { "rtc" }
    { "measurements" }
  }
  { "leapsectz" = "right/UTC" }
  { "broadcast"
    { "interval" = "10" }
    { "address" = "192.168.1.255" }
  }
  { "broadcast"
    { "interval" = "10" }
    { "address" = "192.168.100.255" }
    { "port" = "123" }
  }
  {  }
  { "fallbackdrift"
    { "min" = "16" }
    { "max" = "19" }
  }
  { "mailonchange"
    { "emailaddress" = "root@localhost" }
    { "threshold" = "0.5" }
  }
  { "maxchange"
    { "threshold" = "1000" }
    { "delay" = "1" }
    { "limit" = "2" }
  }
  { "initstepslew"
    { "threshold" = "30" }
    { "address" = "foo.bar.com" }
  }
  { "initstepslew"
    { "threshold" = "30" }
    { "address" = "foo.bar.com" }
    { "address" = "baz.quz.com" }
  }


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
