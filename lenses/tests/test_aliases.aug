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
    { "1" { "name" = "mailer-daemon" }
          { "values" { "1" = "postmaster" } } }
    { "2" { "name" = "postmaster" }
          { "values" { "1" = "root" } } }
    {} {}
    { "3" { "name" = "bin" }
          { "values" { "1" = "root" }
                     { "2" = "adm" } } }
    { "4" { "name" = "daemon" }
          { "values" { "1" = "root" } } }
    { "5" { "name" = "adm" }
          { "values" { "1" = "root" } } }

  test Aliases.lns put file after
      rm "4" ; rm "5" ; 
      set "1/values/10000" "barbar" ;
      set "3/values/2" "ruth"
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
      rm "3" ;
      set "3/values/2" "ruth"
    = *

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
