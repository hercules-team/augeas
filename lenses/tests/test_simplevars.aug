(*
Module: Test_Simplevars
  Provides unit tests and examples for the <Simplevars> lens.
*)

module Test_Simplevars =

(* Variable: conf *)
let conf = "# this is a comment

mykey = myvalue
"

(* Test: Simplevars.lns *)
test Simplevars.lns get conf =
   { "#comment" = "this is a comment" }
   { }
   { "mykey" = "myvalue" }
