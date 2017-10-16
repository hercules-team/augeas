module Test_rancid =

(* Examples from router.db(5) *)
let rancid = "dial1.paris;cisco;up
core1.paris;cisco;down;in testing until 5/5/2001.
core2.paris;cisco;ticketed;Ticket 6054234, 5/3/2001
border1.paris;juniper;up;
"

test Rancid.lns get rancid =
  { "device" = "dial1.paris"
    { "type" = "cisco" }
    { "state" = "up" }
  }
  { "device" = "core1.paris"
    { "type" = "cisco" }
    { "state" = "down" }
    { "comment" = "in testing until 5/5/2001." }
  }
  { "device" = "core2.paris"
    { "type" = "cisco" }
    { "state" = "ticketed" }
    { "comment" = "Ticket 6054234, 5/3/2001" }
  }
  { "device" = "border1.paris"
    { "type" = "juniper" }
    { "state" = "up" }
    { "comment" = "" }
  }

