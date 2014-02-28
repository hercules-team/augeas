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
let dels       = Util.del_str

let colon      = Sep.colon
let comma      = Sep.comma

let sto_to_spc = store Rx.space_in
let sto_to_col  = store /[^:\n]+/
let sto_to_eol = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/

let word    = Rx.word
let password = /[A-Za-z0-9_.!*-]*/
let integer = Rx.integer

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let user      = [ label "user" . store word ]
let user_list = Build.opt_list user comma
let params    = [ label "password" . store password  . colon ]
                . [ label "gid"      . store integer . colon ]
                . user_list?
let entry     = Build.key_value_line word colon params

(* A NIS entry has nothing bar the +@::: bits. *)
let nisentry =
  let nisuser = /\+\@[A-Za-z0-9_.-]+/ in
  let colons = ":::" in
  [ dels "+@" . label "@nis" . store word . dels colons . eol ]

let nisdefault =
  let overrides =
        colon
      . [ label "password" . store word?    . colon ]
      . [ label "gid"      . store integer? . colon ]
      . user_list? in
  [ dels "+" . label "@nisdefault" . overrides? . eol ]


(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = (comment|empty|entry|nisentry|nisdefault) *

let filter     = incl "/etc/group"

let xfm        = transform lns filter
