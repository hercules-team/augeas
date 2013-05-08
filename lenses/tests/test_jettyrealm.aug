(*
Module: Test_JettyRealm
  Provides unit tests and examples for the <JettyRealm> lens.
*)

module Test_JettyRealm =

(* Variable: conf *)
let conf = "### Comment
admin: admin, admin
"

(* Variable: conf_norealm *)
let conf_norealm = "### Comment
admin: admin
"

(* Variable: new_conf *) 
let new_conf = "### Comment
admin: password, admin
"

let lns = JettyRealm.lns 

(* Test: JettyRealm.lns  
 * Get test against tree structure
*)
test lns get conf = 
  { "#comment" = "## Comment" }
  { "user" 
    { "username" = "admin" }
    { "password" = "admin" }
    { "realm" = "admin" }
  }

(* Test: JettyRealm.lns  
 * Get test against tree structure without a realm
*)
test lns get conf_norealm = 
  { "#comment" = "## Comment" }
  { "user" 
    { "username" = "admin" }
    { "password" = "admin" }
  }

(* Test: JettyRealm.lns  
 * Put test changing password to password
*)
test lns put conf after set "/user/password" "password" = new_conf

(* vim: set ts=4  expandtab  sw=4: *)
