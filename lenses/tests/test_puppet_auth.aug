(*
Module: Test_Puppet_Auth
  Provides unit tests and examples for the <Puppet_Auth> lens.
*)

module Test_Puppet_Auth =

(* Variable: full *)
let full = "path ~ ^/file_(metadata|content)/user_files/
# Set environments
environment production, development
environment foo
method  find, search
auth    yes
method save
        allow /^(.+\.)?example.com$/
  allow_ip 192.168.100.0/24 # Added in Puppet 3.0.0
# This overrides the previous auth
  authenticated any
"

(* Test: Puppet_Auth.lns *)
test Puppet_Auth.lns get full =
  { "path" = "^/file_(metadata|content)/user_files/" { "operator" = "~" }
    { "#comment" = "Set environments" }
    { "environment"
      { "1" = "production" }
      { "2" = "development" } }
    { "environment"
      { "3" = "foo" } }
    { "method"
      { "1" = "find" }
      { "2" = "search" } }
    { "auth" = "yes" }
    { "method"
      { "3" = "save" } }
    { "allow"
      { "1" = "/^(.+\.)?example.com$/" } }
    { "allow_ip"
      { "1" = "192.168.100.0/24" }
      { "#comment" = "Added in Puppet 3.0.0" } }
    { "#comment" = "This overrides the previous auth" }
    { "auth" = "any" } }
