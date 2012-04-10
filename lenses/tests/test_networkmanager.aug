(*
Module: Test_NetworkManager
  Provides unit tests and examples for the <NetworkManager> lens.
*)

module Test_NetworkManager =

(* Variable: conf *)
let conf = "[connection]
id=wifoobar
uuid=16fa8830-cf15-4523-8c1f-c6c635246855
type=802-11-wireless

[802-11-wireless]
ssid=wifoobar
mode=infrastructure
mac-address=11:00:99:33:33:AA
security=802-11-wireless-security

[802-11-wireless-security]
key-mgmt=none
wep-key0=123abc123abc

[ipv4]
method=auto

[ipv6]
method=auto\n"

(* Test: NetworkManager.lns *)
test NetworkManager.lns get conf =
  { "connection"
    { "id" = "wifoobar" }
    { "uuid" = "16fa8830-cf15-4523-8c1f-c6c635246855" }
    { "type" = "802-11-wireless" }
    {  }
  }
  { "802-11-wireless"
    { "ssid" = "wifoobar" }
    { "mode" = "infrastructure" }
    { "mac-address" = "11:00:99:33:33:AA" }
    { "security" = "802-11-wireless-security" }
    {  }
  }
  { "802-11-wireless-security"
    { "key-mgmt" = "none" }
    { "wep-key0" = "123abc123abc" }
    {  }
  }
  { "ipv4"
    { "method" = "auto" }
    {  }
  }
  { "ipv6"
    { "method" = "auto" }
  }

