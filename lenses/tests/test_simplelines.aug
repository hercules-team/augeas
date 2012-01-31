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

(* Variable: cronallow *)
  let cronallow = "# Test comment
#
user1
another

user2
"

(* Test: cron.allow file *)
  test SimpleLines.lns get cronallow =
    { "#comment" = "Test comment" }
    { }
    { "1" = "user1" }
    { "2" = "another" }
    { }
    { "3" = "user2" }
