(*
Module: Test_SmbUsers
  Provides unit tests and examples for the <SmbUsers> lens.
*)

module Test_SmbUsers =

(* Variable: conf *)
let conf = "# this is a comment

jarwin = JosephArwin
manderso = MarkAnderson MarkusAndersonus
users = @account
nobody = *
"

(* Test: Simplevars.lns *)
test SmbUsers.lns get conf =
   { "#comment" = "this is a comment" }
   { }
   { "jarwin"
     { "username" = "JosephArwin" } }
   { "manderso"
     { "username" = "MarkAnderson" }
     { "username" = "MarkusAndersonus" } }
   { "users"
     { "username" = "@account" } }
   { "nobody"
     { "username" = "*" } }
