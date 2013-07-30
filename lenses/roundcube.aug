(*
Module: Roundcube
  Parses /etc/roundcube/main.inc.php

Author: Davide Guerri <davide.guerri@gmail.com>

About: Reference
  This lens tries to keep as close as possible to `man 5 keepalived.conf` where possible.

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/roundcube/main.inc.php. See <filter>.

About: Examples
   Please see <Test_Roundcube> file.
*)

module Roundcube =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

(* View: chr_key
    Allowed chars for hash keys in the roundcube main configuration file
*)
let chr_key = /[A-Za-z0-9'"_:-]+/

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

(* View: rcmail_init
    Initialization of rcmail_config hash.
*)
let rcmail_init = [ key /\$rcmail_config/
                 . (Phpvars.sep_eq . Phpvars.sto_to_scl)
                 . Phpvars.sep_scl
                 . Phpvars.eol_or_comment ]
                 
(* View: rcmail_value
   The rcmail_config entries
*)
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

(* View: lns
    The Roundcube lens *)
let lns        = Phpvars.open_php . (Phpvars.empty|Util.comment_multiline|Phpvars.comment_one_line|variable)*

(* Variable: filter *)
let filter     = incl "/etc/roundcube/main.inc.php"

let xfm        = transform lns filter
