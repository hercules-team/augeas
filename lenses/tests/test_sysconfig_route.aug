(*
Module: Test_Sysconfig_Route
  Provides unit tests and examples for the <Sysconfig_Route> lens.
*)
module Test_sysconfig_route =

(* Test: Sysconfig_Route.lns *)
test Sysconfig_Route.lns get "10.40.11.102/32 via 10.40.8.1\n10.1.8.0/24 via 10.40.8.254\n" =
{ "10.40.8.1" = "10.40.11.102/32" }
{ "10.40.8.254" = "10.1.8.0/24" }

(* Test: Sysconfig_Route.lns *)
test Sysconfig_Route.lns get "10.40.11.102/32 via 10.40.8.1\n10.1.8.0/24 via 10.40.8.1\n" =
{ "10.40.8.1" = "10.40.11.102/32" }
{ "10.40.8.1" = "10.1.8.0/24" }
