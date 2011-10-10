module Pass_cont_line =

(* Parse a list of words where the list can stretch over multiple lines.
   Mostly there to demonstrate how to deal with continuation lines. *)

let list_elt = [ label "element" . store /[a-z]+/ ]

let ws_cont = /([ \t]+|[ \t]*\\\\\n[ \t]*)/

let sep = del ws_cont " "
let eol = del /[ \t]*\n/ "\n"

let list = list_elt . ( sep . list_elt )* . eol

let exp_tree = { "element" = "a" } { "element" = "b" }

test list get "a  b\n"      = exp_tree
test list get "a  \\\n b\n" = exp_tree
test list get "a\\\nb\n"    = exp_tree
