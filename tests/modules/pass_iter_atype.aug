module Pass_iter_atype =

  (* Similar to the Pass_concat_atype check; verify that the          *)
  (* typechecker takes tree values into account in the PUT direction, *)
  (* and not just tree labels.                                        *)

  let a (r:regexp) = [ key /a/ . store r ]
  let aa = (a /1/) . (a /2/)
  let lns = ((a /3/) | aa)*


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
