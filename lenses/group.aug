(* Group module for Augeas
 Author: Free Ekanayaka <free@64studio.com>

 Reference: man 5 group

*)

module Group =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol        = Util.eol
let comment    = Util.comment
let empty      = Util.empty

let colon      = del ":" ":"
let comma      = del "," ","

let sto_to_eol = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/

let word       = /[A-Za-z0-9_.-]+/
let integer    = /[0-9]+/

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let user      = [ label "user"     . store word ]
let entry     = [ key word
                . colon
                . [ label "password" . store word    . colon ]
                . [ label "gid"      . store integer . colon ]
                . (user . (comma . user)*)?
                . eol ]

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = (comment|empty|entry) *

let filter
               = incl "/etc/group"
               . Util.stdexcl

let xfm        = transform lns filter
