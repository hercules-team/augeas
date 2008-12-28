module Test_soma =

let conf = "# comment
User = soma

OptionsItem = \"-msglevel avsync=5 -ao jack -af volnorm -novideo -noconsolecontrols -nojoystick -nolirc -nomouseinput\"
Debug = 3
"

test Soma.lns get conf =
   { "#comment" = "comment" }
   { "User" = "soma" }
   {}
   { "OptionsItem" = "\"-msglevel avsync=5 -ao jack -af volnorm -novideo -noconsolecontrols -nojoystick -nolirc -nomouseinput\"" }
   { "Debug" = "3" }
