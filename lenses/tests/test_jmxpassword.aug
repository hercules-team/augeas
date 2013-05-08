(*
Module: Test_JMXPassword
  Provides unit tests and examples for the <JMXPassword> lens.
*)

module Test_JMXPassword =

(* Variable: conf *)
let conf = "# Comment
admin activemq
"

(* Variable: new_conf *) 
let new_conf = "# Comment
admin password
" 

let lns = JMXPassword.lns 

(* Test: JMXPassword.lns  
 * Get test against tree structure
*)
test lns get conf = 
  { "#comment" = "Comment" }
  { "user" 
    { "username" = "admin" }
    { "password" = "activemq" }
  }

(* Test: JMXPassword.lns  
 * Put test changing password to password
*)
test lns put conf after set "/user/password" "password" = new_conf

(* vim: set ts=4  expandtab  sw=4: *)
