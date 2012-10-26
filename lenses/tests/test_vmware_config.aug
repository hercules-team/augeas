(*
Module: Test_VMware_Config
  Provides unit tests and examples for the <VMware_Config> lens.
*)

module Test_VMware_Config =

(* Variable: conf *)
let conf = "libdir = \"/usr/lib/vmware\"
dhcpd.fullpath = \"/usr/bin/vmnet-dhcpd\"
authd.fullpath = \"/usr/sbin/vmware-authd\"
authd.client.port = \"902\"
loop.fullpath = \"/usr/bin/vmware-loop\"
vmware.fullpath = \"/usr/bin/vmware\"
control.fullpath = \"/usr/bin/vmware-cmd\"
serverd.fullpath = \"/usr/sbin/vmware-serverd\"
wizard.fullpath = \"/usr/bin/vmware-wizard\"
serverd.init.fullpath = \"/usr/lib/vmware/serverd/init.pl\"
serverd.vpxuser = \"vpxuser\"
serverd.snmpdconf.subagentenabled = \"TRUE\"
template.useFlatDisks = \"TRUE\"
autoStart.defaultStartDelay = \"60\"
autoStart.enabled = \"True\"
autoStart.defaultStopDelay = \"60\"
"

(* Test: VMware_Config.lns *)
test VMware_Config.lns get conf =
  { "libdir" = "/usr/lib/vmware" }
  { "dhcpd.fullpath" = "/usr/bin/vmnet-dhcpd" }
  { "authd.fullpath" = "/usr/sbin/vmware-authd" }
  { "authd.client.port" = "902" }
  { "loop.fullpath" = "/usr/bin/vmware-loop" }
  { "vmware.fullpath" = "/usr/bin/vmware" }
  { "control.fullpath" = "/usr/bin/vmware-cmd" }
  { "serverd.fullpath" = "/usr/sbin/vmware-serverd" }
  { "wizard.fullpath" = "/usr/bin/vmware-wizard" }
  { "serverd.init.fullpath" = "/usr/lib/vmware/serverd/init.pl" }
  { "serverd.vpxuser" = "vpxuser" }
  { "serverd.snmpdconf.subagentenabled" = "TRUE" }
  { "template.useFlatDisks" = "TRUE" }
  { "autoStart.defaultStartDelay" = "60" }
  { "autoStart.enabled" = "True" }
  { "autoStart.defaultStopDelay" = "60" }

(* Test: VMware_Config.lns
   Quotes are not mandatory *)
test VMware_Config.lns get "xkeymap.nokeycodeMap = true\n" =
  { "xkeymap.nokeycodeMap" = "true" }
