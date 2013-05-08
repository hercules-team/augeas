(*
Module: Test_JMXAccess
  Provides unit tests and examples for the <JMXAccess> lens.
*)

module Test_JMXAccess =

(* Variable: conf *)
let conf = "# Comment
admin readwrite
"

(* Variable: new_conf *) 
let new_conf = "# Comment
admin readonly
" 

let lns = JMXAccess.lns 

(* Test: JMXAccess.lns  
 * Get test against tree structure
*)
test lns get conf = 
  { "#comment" = "Comment" }
  { "user" 
    { "username" = "admin" }
    { "access" = "readwrite" }
  }

(* Test: JMXAccess.lns  
 * Put test changing access to readonly
*)
test lns put conf after set "/user/access" "readonly" = new_conf

(* vim: set ts=4  expandtab  sw=4: *)
