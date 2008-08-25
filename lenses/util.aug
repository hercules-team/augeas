(* Simple utilities used in several places *)
module Util =
  let del_str (s:string) = del s s

  let del_ws = del /[ \t]+/
  let del_ws_spc = del_ws " "
  let del_ws_tab = del_ws "\t"

  let del_opt_ws = del /[ \t]*/

  let eol = del /[ \t]*\n/ "\n"
  let indent = del /[ \t]*/ ""

  (* comment and empty
     This is a general definition of comment and empty.
     It allows indentation for comments, removes the leading and trailing spaces
     of comments and stores them in nodes, except for empty comments which are 
     ignored together with empty lines
  *)
  let comment = 
    [ indent . label "#comment" . del /#[ \t]*/ "# " 
        . store /([^ \t\n].*[^ \t\n]|[^ \t\n])/ . eol ]

  let empty   = [ del /[ \t]*#?[ \t]*\n/ "" ]

  (* Split (SEP . ELT)* into an array-like tree where each match for ELT *)
  (* appears in a separate subtree. The labels for the subtrees are      *)
  (* consecutive numbers, starting at 0                                  *)
  let split (elt:lens) (sep:lens) =
    let sym = gensym "split" in
    counter sym . ( [ seq sym . sep . elt ] ) *

  (* Exclusion for files that are commonly not wanted/needed              *)
  let stdexcl = (excl "*~") . (excl "*.rpmnew") . (excl "*.rpmsave")

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
