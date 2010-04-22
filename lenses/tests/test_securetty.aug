module Test_securetty =

 (* basic test *)
 let basic = "tty0\ntty1\ntty2\n"

 (* declare the lens to test and the resulting tree *)
 test Securetty.lns get basic =
    { "1" = "tty0" }
    { "2" = "tty1" }
    { "3" = "tty2" }

 (* complete test *)
 let complete = "# some comment

tty0
# X11 display
:0.0

console  # allow root from console
"

 (* declare the lens to test and the resulting tree *)
 test Securetty.lns get complete =
    { "#comment" = "some comment" }
    {}
    { "1" = "tty0" }
    { "#comment" = "X11 display" }
    { "2" = ":0.0" }
    {}
    { "3" = "console"
      { "#comment" = "allow root from console" } }
