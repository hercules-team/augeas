(*
Module: Test_Postfix_Passwordmap
  Provides unit tests and examples for the <Postfix_Passwordmap> lens.
*)

module Test_Postfix_Passwordmap =

(* View: conf *)
let conf = "# comment
*                               username:password
[mail.isp.example]              username:password
[mail.isp.example]:submission   username:password
[mail.isp.example]:587          username:password
mail.isp.example                username:password
user@mail.isp.example           username:
mail.isp.example
        username2:password2
"

(* Test: Postfix_Passwordmap.lns *)
test Postfix_Passwordmap.lns get conf =
  { "#comment" = "comment" }
  { "pattern" = "*"
    { "username" = "username" }
    { "password" = "password" } }
  { "pattern" = "[mail.isp.example]"
    { "username" = "username" }
    { "password" = "password" } }
  { "pattern" = "[mail.isp.example]:submission"
    { "username" = "username" }
    { "password" = "password" } }
  { "pattern" = "[mail.isp.example]:587"
    { "username" = "username" }
    { "password" = "password" } }
  { "pattern" = "mail.isp.example"
    { "username" = "username" }
    { "password" = "password" } }
  { "pattern" = "user@mail.isp.example"
    { "username" = "username" }
    { "password" } }
  { "pattern" = "mail.isp.example"
    { "username" = "username2" }
    { "password" = "password2" } }
