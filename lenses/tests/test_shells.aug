(* Test for shells lens *)

module Test_shells =

   let conf = "# this is a comment

/bin/bash
/bin/tcsh
"

   test Shells.lns get conf =
      { "#comment" = "this is a comment" }
      {}
      { "1" = "/bin/bash" }
      { "2" = "/bin/tcsh" }
