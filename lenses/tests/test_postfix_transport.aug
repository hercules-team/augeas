(*
Module: Test_Postfix_Transport
  Provides unit tests and examples for the <Postfix_Transport> lens.
*)

module Test_Postfix_Transport =

(* View: conf *)
let conf = "# a comment
the.backed-up.domain.tld       relay:[their.mail.host.tld]
.my.domain   :
*            smtp:outbound-relay.my.domain
example.com      uucp:example
example.com      slow:
example.com      :[gateway.example.com]
user.foo@example.com  
    smtp:bar.example:2025
.example.com     error:mail for *.example.com is not deliverable
"

(* Test: Postfix_Transport.lns *)
test Postfix_Transport.lns get conf =
  { "#comment" = "a comment" }
  { "pattern" = "the.backed-up.domain.tld"
    { "transport" = "relay" }
    { "nexthop" = "[their.mail.host.tld]" } }
  { "pattern" = ".my.domain"
    { "transport" }
    { "nexthop" } }
  { "pattern" = "*"
    { "transport" = "smtp" }
    { "nexthop" = "outbound-relay.my.domain" } }
  { "pattern" = "example.com"
    { "transport" = "uucp" }
    { "nexthop" = "example" } }
  { "pattern" = "example.com"
    { "transport" = "slow" }
    { "nexthop" } }
  { "pattern" = "example.com"
    { "transport" }
    { "nexthop" = "[gateway.example.com]" } }
  { "pattern" = "user.foo@example.com"
    { "transport" = "smtp" }
    { "nexthop" = "bar.example:2025" } }
  { "pattern" = ".example.com"
    { "transport" = "error" }
    { "nexthop" = "mail for *.example.com is not deliverable" } }

(* Test: Postfix_Transport.lns
     Bug #303 *)
test Postfix_Transport.lns get "user@example.com [12.34.56.78]:587\n" =
  { "pattern" = "user@example.com"
    { "host" = "[12.34.56.78]" }
    { "port" = "587" } }
