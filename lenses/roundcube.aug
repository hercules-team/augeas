(* Roundcube Configuration

   Davide Guerri <davide.guerri@gmail.com>

*)

module Roundcube =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let chr_key = /[A-Za-z0-9'"_:-]+/

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let rcmail_init = [ key /\$rcmail_config/
                 . (Phpvars.sep_eq . Phpvars.sto_to_scl)
                 . Phpvars.sep_scl
                 . Phpvars.eol_or_comment ]
                 
let rcmail_value = [ del /\$rcmail_config\['/ "$rcmail_config['"
                 . key chr_key
                 . Util.del_str "']"
                 . (Phpvars.sep_eq . Phpvars.sto_to_scl)
                 . Phpvars.sep_scl
                 . Phpvars.eol_or_comment ]

let variable = Util.indent . (rcmail_init|rcmail_value)


(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = Phpvars.open_php . (Phpvars.empty|Util.comment_multiline|Phpvars.comment_one_line|variable)*

let filter     = incl "/etc/roundcube/main.inc.php"

let xfm        = transform lns filter
