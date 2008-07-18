(* Dput module for Augeas                     *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)
(* Status: most settings supported            *)


module Dput =
  autoload xfm


    (* Define useful shortcuts *)

    let eol = Util.del_str "\n"
    let del_to_eol = del /[^\n]*/ ""
    let value_sep = del /[ \t]*=[ \t]*/ "="
    let value_to_eol = store /([^ \t\n][^\n]*)?/


    (* Define kw_arg function *)

    let kw_arg (kw:string) = [ key kw . value_sep . value_to_eol . eol ]


    (* Define comment and empty strings *)

    let comment = [ label "comment" . del /#[ \t]*/ "#" .  store /([^ \t\n][^\n]*)?/ . eol ]
    let empty  = [ del /[ \t]*/ "" . eol ]


    (* Define record *)

    let target = Util.del_str "[" . store /[^]= ]+/ . Util.del_str "]". eol

    let settings = kw_arg "allow_non-us_software"
                 | kw_arg "allow_unsigned_uploads"
                 | kw_arg "check_version"
                 | kw_arg "default_host_main"
                 | kw_arg "default_host_non-us"
                 | kw_arg "fqdn"
                 | kw_arg "hash"
                 | kw_arg "incoming"
                 | kw_arg "login"
                 | kw_arg "method"
                 | kw_arg "passive_ftp"
                 | kw_arg "post_upload_command"
                 | kw_arg "pre_upload_command"
                 | kw_arg "run_dinstall"
                 | kw_arg "run_lintian"
                 | kw_arg "scp_compress"


    let record = [ label "target" . target . settings* ]


    (* Define lens *)

    let lns = ( comment | record | empty )*

    let filter = (incl "/etc/dput.cf")
        . (incl "~/.dput.cf")
        . Util.stdexcl

    let xfm = transform lns filter
