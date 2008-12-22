(* Postfix_Main module for Augeas
 Author: Free Ekanayaka <free@64studio.com>

 Reference:


*)

module Postfix_Main =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol        = Util.eol
let spc        = Util.del_ws_spc
let indent     = del /[ \t]*/ " "
let comment    = Util.comment
let empty      = Util.empty
let eq         = del /[ \t]*=/ " ="

let sto_to_eol = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/
let sto_to_spc = store /[^ \t\n]+/

let word       = /[A-Za-z0-9_.-]+/
let value      = [ key word . spc . sto_to_eol . eol ]

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let entry     = [ key word . eq . (indent . sto_to_eol)? . eol ]

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = (comment|empty|entry) *

let filter     = incl "/etc/postfix/main.cf"
               . Util.stdexcl

let xfm        = transform lns filter
