(* Passwd module for Augeas
 Author: Free Ekanayaka <free@64studio.com>

 Reference: man 5 passwd

*)

module Passwd =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol        = Util.eol
let comment    = Util.comment
let empty      = Util.empty

let colon      = del /:/ ":"

let sto_to_col  = store /[^:\n]+/
let sto_to_eol = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/

let word       = /[A-Za-z0-9_.-]+/
let integer    = /[0-9]+/

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let entry     = [ key word
                . colon
                . [ label "password" . store word    . colon ]
                . [ label "uid"      . store integer . colon ]
                . [ label "gid"      . store integer . colon ]
                . [ label "name"     . sto_to_col?   . colon ]
                . [ label "home"     . sto_to_col?  . colon ]
                . [ label "shell"    . sto_to_eol? ]
                . eol ]

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = (comment|empty|entry) *

let filter
               = incl "/etc/passwd"
               . Util.stdexcl

let xfm        = transform lns filter
