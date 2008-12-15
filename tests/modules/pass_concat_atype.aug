module Pass_concat_atype =

  (* This passes both the ctype and atype check for unambiguous  *)
  (* concatenation because the STORE's keep everything copacetic.*)
  (* If we would only look at tree labels, we'd get a type error *)
  (* in the PUT direction, because we couldn't tell how to split *)
  (* the tree                                                    *)
  (*    { "a" = .. } { "b" = .. } { "a" = .. }                   *)
  (* solely by looking at tree labels.                           *)

  let ab = [ key /a/ . store /1/ ] . ([ key /b/ . store /2/ ]?)
  let ba = ([ key /b/ . store /3/ ])? . [ key /a/ . store /4/ ]
  let lns = ab . ba


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
