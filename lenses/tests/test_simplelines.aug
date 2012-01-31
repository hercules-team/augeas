(*
Module: Test_Simplelines
  Provides unit tests and examples for the <Simplelines> lens.
*)

module Test_Simplelines =

(* Variable: conf *)
let conf = "# This is a comment

word
a line
  indented line
with $péci@l cH@r2ct3rs
"

(* Test: Simplelines.lns *)
test Simplelines.lns get conf =
   { "#comment" = "This is a comment" }
   { }
   { "1" = "word" }
   { "2" = "a line" }
   { "3" = "indented line" }
   { "4" = "with $péci@l cH@r2ct3rs" }
