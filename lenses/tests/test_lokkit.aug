module Test_lokkit =

let conf = "# Configuration file for system-config-firewall

--enabled
--port=111:tcp
-p 111:udp
-p 2020-2049:tcp
--port=5900-5910:tcp
--custom-rules=ipv4:filter:/var/lib/misc/iptables-forward-bridged
-s dns
--service=ssh
--trust=trust1
--masq=eth42
--block-icmp=5
-t trust0
--addmodule=fancy
--removemodule=broken
--forward-port=if=forw0:port=42:proto=tcp:toport=42:toaddr=192.168.0.42
--selinux=permissive
"

test Lokkit.lns get conf =
  { "#comment" = "Configuration file for system-config-firewall" }
  { }
  { "enabled" }
  { "port"
    { "start" = "111" }
    { "protocol" = "tcp" } }
  { "port"
    { "start" = "111" }
    { "protocol" = "udp" } }
  { "port"
    { "start" = "2020" }
    { "end" = "2049" }
    { "protocol" = "tcp" } }
  { "port"
    { "start" = "5900" }
    { "end" = "5910" }
    { "protocol" = "tcp" } }
  { "custom-rules" = "/var/lib/misc/iptables-forward-bridged"
    { "type" = "ipv4" }
    { "table" = "filter" } }
  { "service" = "dns" }
  { "service" = "ssh" }
  { "trust" = "trust1" }
  { "masq" = "eth42" }
  { "block-icmp" = "5" }
  { "trust" = "trust0" }
  { "addmodule" = "fancy" }
  { "removemodule" = "broken" }
  { "forward-port"
    { "if" = "forw0" }
    { "port" = "42" }
    { "proto" = "tcp" }
    { "toport" = "42" }
    { "toaddr" = "192.168.0.42" } }
  { "selinux" = "permissive" }

test Lokkit.custom_rules get
"--custom-rules=ipv4:filter:/some/file\n" =
  { "custom-rules" = "/some/file"
    { "type" = "ipv4" }
    { "table" = "filter" } }

test Lokkit.custom_rules get
"--custom-rules=filter:/some/file\n" =
  { "custom-rules" = "/some/file"
    { "table" = "filter" } }

test Lokkit.custom_rules get
"--custom-rules=ipv4:/some/file\n" =
  { "custom-rules" = "/some/file"
    { "type" = "ipv4" } }

test Lokkit.custom_rules get
"--custom-rules=/some/file\n" =
  { "custom-rules" = "/some/file" }

test Lokkit.lns get
"--trust=tun+\n--trust=eth0.42\n--trust=eth0:1\n" =
  { "trust" = "tun+" }
  { "trust" = "eth0.42" }
  { "trust" = "eth0:1" }

(* We didn't allow '-' in the service name *)
test Lokkit.lns get "--service=samba-client\n" =
  { "service" = "samba-client" }
