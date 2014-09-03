(*
Module: Test_Aliases
  Provides unit tests and examples for the <Aliases> lens.
*)

module Test_aliases =

(* Variable: file
   A full configuration file *)
  let file = "#
#  Aliases in this file will NOT be expanded in the header from
#  Mail, but WILL be visible over networks or from /bin/mail.

# Basic system aliases -- these MUST be present.
mailer-daemon:	postmaster
postmaster:	root

# General redirections for pseudo accounts.
bin:		root , adm,
  bob
daemon:		root
adm:		root
file:		/var/foo
pipe1:		|/bin/ls
pipe2 :		|\"/usr/bin/ls args,\"
"

(* Test: Aliases.lns
   Testing <Aliases.lns> on <file> *)
  test Aliases.lns get file =
    { }
    { "#comment" = "Aliases in this file will NOT be expanded in the header from" }
    { "#comment" = "Mail, but WILL be visible over networks or from /bin/mail." }
    {}
    { "#comment" = "Basic system aliases -- these MUST be present." }
    { "1" { "name" = "mailer-daemon" }
          { "value" = "postmaster" } }
    { "2" { "name" = "postmaster" }
          { "value" = "root" } }
    {}
    { "#comment" = "General redirections for pseudo accounts." }
    { "3" { "name" = "bin" }
          { "value" = "root" }
          { "value" = "adm" }
          { "value" = "bob" } }
    { "4" { "name" = "daemon" }
          { "value" = "root" } }
    { "5" { "name" = "adm" }
          { "value" = "root" } }
    { "6" { "name" = "file" }
          { "value" = "/var/foo" } }
    { "7" { "name" = "pipe1" }
          { "value" = "|/bin/ls" } }
    { "8" { "name" = "pipe2" }
          { "value" = "|\"/usr/bin/ls args,\"" } }

(* Test: Aliases.lns
   Put test for <Aliases.lns> on <file> *)
  test Aliases.lns put file after
    rm "/4" ; rm "/5" ; rm "/6" ; rm "/7" ; rm "/8" ;
      set "/1/value[2]" "barbar" ;
      set "/3/value[2]" "ruth"
    = "#
#  Aliases in this file will NOT be expanded in the header from
#  Mail, but WILL be visible over networks or from /bin/mail.

# Basic system aliases -- these MUST be present.
mailer-daemon:	postmaster, barbar
postmaster:	root

# General redirections for pseudo accounts.
bin:		root , ruth,
  bob
"

  (* Test: Aliases.lns
     Schema violation, no 3/name *)
  test Aliases.lns put file after
      rm "/3" ;
      set "/3/value/2" "ruth"
    = *

  (* Variable: nocomma
     Don't have to have whitespace after a comma *)
  let nocomma = "alias: target1,target2\n"

  (* Test: Aliases.lns
     Testing <Aliases.lns> on <nocomma> *)
  test Aliases.lns get nocomma =
    { "1"
        { "name" = "alias" }
        { "value" = "target1" }
        { "value" = "target2" } }

  (* Test: Aliases.lns
     Ticket #229: commands can be fully enclosed in quotes *)
  test Aliases.lns get "somebody: \"|exit 67\"\n" =
    { "1"
        { "name" = "somebody" }
        { "value" = "\"|exit 67\"" } }

  (* Test: Aliases.lns
     Don't have to have whitespace after the colon *)
  test Aliases.lns get "alias:target\n" =
    { "1"
        { "name" = "alias" }
        { "value" = "target" } }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
