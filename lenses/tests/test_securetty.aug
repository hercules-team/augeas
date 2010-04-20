module Test_securetty =

 (* basic test *)
 let basic = "tty0\ntty1\ntty2\n"

 (* declare the lens to test and the resulting tree *)
 test Securetty.lns get basic =
    { "1" = "tty0" }
    { "2" = "tty1" }
    { "3" = "tty2" }
