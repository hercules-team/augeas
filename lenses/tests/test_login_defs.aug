(*
Module: Test_login_defs
  Test cases for the login_defs lense

Author: Erinn Looney-Triggs

About: License
  This file is licensed under the LGPLv2+, like the rest of Augeas.
*)
module Test_login_defs =

let record = "MAIL_DIR        /var/spool/mail
ENCRYPT_METHOD SHA512 
UMASK           077
"

test Login_defs.lns get record =
    { "MAIL_DIR" = "/var/spool/mail" } 
    { "ENCRYPT_METHOD" = "SHA512" }
    { "UMASK" = "077" }

let comment ="# *REQUIRED*
"

test Login_defs.lns get comment = 
  {"#comment" = "*REQUIRED*"}
