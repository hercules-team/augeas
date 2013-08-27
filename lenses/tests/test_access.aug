(*
Module: Test_Access
  Provides unit tests and examples for the <Access> lens.
*)

module Test_access =

(* Variable: conf
    A full configuration *)
let conf = "+ : ALL : LOCAL
+ : root : localhost.localdomain
- : root : 127.0.0.1 .localdomain
+ : root alice@server1 @admins (wheel) : cron crond :0 tty1 tty2 tty3 tty4 tty5 tty6
# IP v6 support
+ : john foo : 2001:4ca0:0:101::1 2001:4ca0:0:101::/64
# Except
+ : ALL EXCEPT john @wheel : ALL EXCEPT LOCAL .win.tue.nl
# No spaces
+:root:.example.com
"

(* Test: Access.lns
     Test the full <conf> *)
test Access.lns get conf =
    { "access" = "+"
        { "user" = "ALL" }
        { "origin" = "LOCAL" } }
    { "access" = "+"
        { "user" = "root" }
        { "origin" = "localhost.localdomain" } }
    { "access" = "-"
        { "user" = "root" }
        { "origin" = "127.0.0.1" }
        { "origin" = ".localdomain" } }
    { "access" = "+"
        { "user" = "root" }
        { "user" = "alice"
           { "host" = "server1" } }
        { "netgroup" = "admins" }
        { "group" = "wheel" }
        { "origin" = "cron" }
        { "origin" = "crond" }
        { "origin" = ":0" }
        { "origin" = "tty1" }
        { "origin" = "tty2" }
        { "origin" = "tty3" }
        { "origin" = "tty4" }
        { "origin" = "tty5" }
        { "origin" = "tty6" } }
    { "#comment" = "IP v6 support" }
    { "access" = "+"
        { "user" = "john" }
        { "user" = "foo" }
        { "origin" = "2001:4ca0:0:101::1" }
        { "origin" = "2001:4ca0:0:101::/64" } }
    { "#comment" = "Except" }
    { "access" = "+"
        { "user" = "ALL" }
        { "except"
           { "user" = "john" }
           { "netgroup" = "wheel" } }
        { "origin" = "ALL" }
        { "except"
           { "origin" = "LOCAL" }
           { "origin" = ".win.tue.nl" } } }
    { "#comment" = "No spaces" }
    { "access" = "+"
        { "user" = "root" }
        { "origin" = ".example.com" } }

test Access.lns put conf after
    insa "access" "access[last()]" ;
    set "access[last()]" "-" ;
    set "access[last()]/user" "ALL" ;
    set "access[last()]/origin" "ALL"
 = "+ : ALL : LOCAL
+ : root : localhost.localdomain
- : root : 127.0.0.1 .localdomain
+ : root alice@server1 @admins (wheel) : cron crond :0 tty1 tty2 tty3 tty4 tty5 tty6
# IP v6 support
+ : john foo : 2001:4ca0:0:101::1 2001:4ca0:0:101::/64
# Except
+ : ALL EXCEPT john @wheel : ALL EXCEPT LOCAL .win.tue.nl
# No spaces
+:root:.example.com
- : ALL : ALL
"

(* Test: Access.lns
     - netgroups (starting with '@') are mapped as "netgroup" nodes;
     - nisdomains (starting with '@@') are mapped as "nisdomain" nodes.

 This closes <ticket #190 at https://fedorahosted.org/augeas/ticket/190>.
 *)
test Access.lns get "+ : @group@@domain : ALL \n" =
  { "access" = "+"
    { "netgroup" = "group"
      { "nisdomain" = "domain" } }
    { "origin" = "ALL" } }

(* Test Access.lns
   Put test for netgroup and nisdomain *)
test Access.lns put "+ : @group : ALL \n" after
  set "/access/netgroup[. = 'group']/nisdomain" "domain" =
"+ : @group@@domain : ALL \n"

(* Check DOMAIN\user style entry *)
test Access.lns get "+ : root : foo1.bar.org foo3.bar.org
+ : (DOMAIN\linux_users) : ALL
+ : DOMAIN\linux_user : ALL\n" =
  { "access" = "+"
    { "user" = "root" }
    { "origin" = "foo1.bar.org" }
    { "origin" = "foo3.bar.org" } }
  { "access" = "+"
    { "group" = "DOMAIN\linux_users" }
    { "origin" = "ALL" } }
  { "access" = "+"
    { "user" = "DOMAIN\linux_user" }
    { "origin" = "ALL" } }
