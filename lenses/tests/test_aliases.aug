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
    {} {} {} {} {}                           (* The comments *)
    { "0" { "name" = "mailer-daemon" }
          { "values" { "0" = "postmaster" } } }
    { "1" { "name" = "postmaster" }
          { "values" { "0" = "root" } } }
    {} {}
    { "2" { "name" = "bin" }
          { "values" { "0" = "root" }
                     { "1" = "adm" } } }
    { "3" { "name" = "daemon" }
          { "values" { "0" = "root" } } }
    { "4" { "name" = "adm" }
          { "values" { "0" = "root" } } }

  test Aliases.lns put file after
      rm "3" ; rm "4" ; 
      set "0/values/10000" "barbar" ;
      set "2/values/1" "ruth"
    = "#
#  Aliases in this file will NOT be expanded in the header from
#  Mail, but WILL be visible over networks or from /bin/mail.

# Basic system aliases -- these MUST be present.
mailer-daemon:	postmaster, barbar
postmaster:	root

# General redirections for pseudo accounts.
bin:		root, ruth
"

  (* Schema violation, no 2/name *)
  test Aliases.lns put file after
      rm "2" ;
      set "2/values/1" "ruth"
    = *

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
