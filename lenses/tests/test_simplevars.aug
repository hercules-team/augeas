(*
Module: Test_Simplevars
  Provides unit tests and examples for the <Simplevars> lens.
*)

module Test_Simplevars =

(* Variable: conf *)
let conf = "# this is a comment

mykey = myvalue # eol comment
anotherkey = another value
"

(* Test: Simplevars.lns *)
test Simplevars.lns get conf =
   { "#comment" = "this is a comment" }
   { }
   { "mykey" = "myvalue"
     { "#comment" = "eol comment" } }
   { "anotherkey" = "another value" }

(* Test: Simplevars.lns
   Quotes are OK in variables that do not begin with a quote *)
test Simplevars.lns get "UserParameter=custom.vfs.dev.read.ops[*],cat /proc/diskstats | grep $1 | head -1 | awk '{print $$4}'\n" =
     { "UserParameter" = "custom.vfs.dev.read.ops[*],cat /proc/diskstats | grep $1 | head -1 | awk '{print $$4}'" }

(* Test: Simplevars.lns
    Support flags *)
test Simplevars.lns get "dnsadminapp\n" =
  { "dnsadminapp" }

(* Test: Simplevars.lns
     Support empty values *)
test Simplevars.lns get "foo =\n" =
  { "foo" = "" { } }
