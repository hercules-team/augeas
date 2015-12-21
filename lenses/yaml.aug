(*
Yaml subset lens
Only valid for the following structure:

defaults: &anchor
  repo1: master

host:
  # Inheritance
  <<: *anchor
  repo2: branch
*)
module YAML =

autoload xfm

let colon = Sep.colon
let space = Sep.space
let val = store Rx.word
let eol = Util.eol
let empty = Util.empty
let comment = Util.comment_noindent

(* the imposed indent is 2 spaces *)
let indent = Util.del_str "  "

(* <<: *anchor *)
let _inherit = [ key "<<" . colon . space . Util.del_str "*" . val . eol ]
let inherit = indent . _inherit . (indent . comment)*

(* { "repo" = "branch" } *)
let _repo = [ key Rx.word . colon . space . val . eol ]
let repo = indent . _repo . (indent . comment)*

(* &anchor *)
let anchor = Util.del_str "&" . val

(*
host:
  # Inheritance
  <<: *anchor
  repo2: branch
*)
let entry = [ key Rx.word . colon . (space . anchor)? . eol
            . (indent . comment)*
            . ((inherit . (repo+)?) | repo+)
            ]

let lns = (entry | comment | empty)+

let filter = incl "/path/to/custom.yml" . Util.stdexcl

let xfm = transform lns filter
