(*
Module: Test_Iscsid
  Provides unit tests and examples for the <Iscsid> lens.
*)

module Test_Iscsid =

(* Variable: conf
    A full configuration *)
let conf = "################
# iSNS settings
################
# Address of iSNS server
isns.address = 127.0.0.1
isns.port = 3260

# *************
# CHAP Settings
# *************

# To enable CHAP authentication set node.session.auth.authmethod
# to CHAP. The default is None.
node.session.auth.authmethod = CHAP

# To set a CHAP username and password for initiator
# authentication by the target(s), uncomment the following lines:
node.session.auth.username = someuser1
node.session.auth.password = somep$31#$^&7!

# To enable CHAP authentication for a discovery session to the target
# set discovery.sendtargets.auth.authmethod to CHAP. The default is None.
discovery.sendtargets.auth.authmethod = CHAP

# To set a discovery session CHAP username and password for the initiator
# authentication by the target(s), uncomment the following lines:
discovery.sendtargets.auth.username = someuser3
discovery.sendtargets.auth.password = _09+7)(,./?;'p[]
"

(* Test: Iscsid.lns
     Test the full <conf> *)
test Iscsid.lns get conf = { "#comment" = "###############" }
  { "#comment" = "iSNS settings" }
  { "#comment" = "###############" }
  { "#comment" = "Address of iSNS server" }
  { "isns.address" = "127.0.0.1" }
  { "isns.port" = "3260" }
  {  }
  { "#comment" = "*************" }
  { "#comment" = "CHAP Settings" }
  { "#comment" = "*************" }
  {  }
  { "#comment" = "To enable CHAP authentication set node.session.auth.authmethod" }
  { "#comment" = "to CHAP. The default is None." }
  { "node.session.auth.authmethod" = "CHAP" }
  {  }
  { "#comment" = "To set a CHAP username and password for initiator" }
  { "#comment" = "authentication by the target(s), uncomment the following lines:" }
  { "node.session.auth.username" = "someuser1" }
  { "node.session.auth.password" = "somep$31#$^&7!" }
  {  }
  { "#comment" = "To enable CHAP authentication for a discovery session to the target" }
  { "#comment" = "set discovery.sendtargets.auth.authmethod to CHAP. The default is None." }
  { "discovery.sendtargets.auth.authmethod" = "CHAP" }
  {  }
  { "#comment" = "To set a discovery session CHAP username and password for the initiator" }
  { "#comment" = "authentication by the target(s), uncomment the following lines:" }
  { "discovery.sendtargets.auth.username" = "someuser3" }
  { "discovery.sendtargets.auth.password" = "_09+7)(,./?;'p[]" }
