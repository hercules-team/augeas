(* Roundcube Configuration

   Davide Guerri <davide.guerri@gmail.com>

   Heavily based on Phpvars module for Augeas (by Free Ekanayaka <free@64studio.com>)

*)

module Roundcube =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol        = Util.eol
let empty      = Util.empty_c_style

let open_php   = del /<\?(php)?[ \t]*\n/i "<?php\n"

let sep_eq     = del /[ \t\n]*=[ \t\n]*/ " = "
let sep_scl    = del /[ \t]*;/ ";"

let chr_key = /[A-Za-z0-9'"_:-]+/

let sto_to_scl = store (/([^ \t\n].*[^ \t\n;]|[^ \t\n;])/ - /.*;[ \t]*(\/\/|#).*/) (* " *)

(************************************************************************
 *                              COMMENTS
 *************************************************************************)

(* Both c-style and shell-style comments are valid
   Default to c-style *)
let comment_one_line = Util.comment_generic /[ \t]*(\/\/|#)[ \t]*/ "// "

let comment_eol = Util.comment_generic /[ \t]*(\/\/|#)[ \t]*/ " // "

let comment      = Util.comment_multiline | comment_one_line

let eol_or_comment = eol | comment_eol


(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let rcmail_init = [ key /\$rcmail_config/
                 . (sep_eq . sto_to_scl)
                 . sep_scl
                 . eol_or_comment ]
                 
let rcmail_value = [ del /\$rcmail_config\['/ "$rcmail_config['"
                 . key chr_key
                 . del /'\]/ "']"
                 . (sep_eq . sto_to_scl)
                 . sep_scl
                 . eol_or_comment ]

let variable = Util.indent . (rcmail_init|rcmail_value)


(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = open_php . (empty|comment|variable)*

let filter     = incl "/etc/roundcube/main.inc.php"
               . Util.stdexcl

let xfm        = transform lns filter