module Test_aliases =

  let file = "#
#  Aliases in this file will NOT be expanded in the header from
#  Mail, but WILL be visible over networks or from /bin/mail.

# Basic system aliases -- these MUST be present.
mailer-daemon:	postmaster
postmaster:	root

# General redirections for pseudo accounts.
bin:		root, adm
daemon:		root
adm:		root
"
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
          { "value" = "adm" } }
    { "4" { "name" = "daemon" }
          { "value" = "root" } }
    { "5" { "name" = "adm" }
          { "value" = "root" } }

  test Aliases.lns put file after
    rm "/4" ; rm "/5" ;
      set "/1/value[2]" "barbar" ;
      set "/3/value[2]" "ruth"
    = "#
#  Aliases in this file will NOT be expanded in the header from
#  Mail, but WILL be visible over networks or from /bin/mail.

# Basic system aliases -- these MUST be present.
mailer-daemon:	postmaster, barbar
postmaster:	root

# General redirections for pseudo accounts.
bin:		root, ruth
"

  (* Schema violation, no 3/name *)
  test Aliases.lns put file after
      rm "/3" ;
      set "/3/value/2" "ruth"
    = *

  (* Don't have to have whitespace after a comma *)
  let nocomma = "alias: target1,target2\n"

  test Aliases.lns get nocomma =
    { "1"
        { "name" = "alias" }
        { "value" = "target1" }
        { "value" = "target2" } }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
