(* Spacevars module for Augeas
 Author: Free Ekanayaka <free@64studio.com>

 Reference: man interfaces
 This is a generic lens for simple key/value configuration files where
 keys and values are separated by a sequence of spaces or tabs.

*)

module Spacevars =
  autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol     = Util.eol
let spc     = Util.del_ws_spc
let comment = Util.comment
let empty   = Util.empty

let sto_to_eol = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/

(************************************************************************
 *                               ENTRIES
 *************************************************************************)


let entry (kw:regexp)
               = [ key kw . spc . sto_to_eol . eol ]
let entry_re   = /[A-Za-z0-9\._-]+(\[[0-9]+\])?/

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns (entry:lens) = (comment|empty|entry) *

let simple_lns = lns (entry entry_re)

(* configuration files that can be parsed without customizing the lens *)
let filter = Util.stdexcl
           . incl "/etc/havp/havp.config"
           . incl "/etc/ldap.conf"
           . incl "/etc/ldap/ldap.conf"

let xfm = transform simple_lns filter
