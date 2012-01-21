(* Test for shells lens *)

module Test_shells =

   let conf = "# this is a comment

/bin/bash
/bin/tcsh
/opt/csw/bin/bash   # CSWbash
"

   test Shells.lns get conf =
      { "#comment" = "this is a comment" }
      {}
      { "1" = "/bin/bash" }
      { "2" = "/bin/tcsh" }
      { "3" = "/opt/csw/bin/bash"
        { "#comment" = "CSWbash" } }
