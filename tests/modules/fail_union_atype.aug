module Fail_union_atype =
  (* This is illegal, otherwise we don't know which alternative *)
  (* to take for a tree { "a" = "?" }                           *)

  let lns = [ key /a/ . store /b/ ] | [ key /a/ . store /c/ ]


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
