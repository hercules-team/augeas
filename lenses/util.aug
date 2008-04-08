(* Simple utilities used in several places *)
module Util =
  let del_str (s:string) = del s s

  let del_ws = del /[ \t]+/
  let del_ws_spc = del_ws " "
  let del_ws_tab = del_ws "\t"

  (* Split (SEP . ELT)* into an array-like tree where each match for ELT *)
  (* appears in a separate subtree. The labels for the subtrees are      *)
  (* consecutive numbers, starting at 0                                  *)
  let split (elt:lens) (sep:lens) =
    let sym = gensym "split" in
    counter sym . ( [ seq sym . sep . elt ] ) *

  (* Exclusion for files that are commonly not wanted/needed              *)
  let stdexcl = excl "*~"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
