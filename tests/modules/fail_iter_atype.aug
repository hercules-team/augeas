module Fail_iter_atype =

  (* Similar to the Fail_concat_atype check *)

  let a (r:regexp) = [ key /a/ . store r ]
  let aa = (a /1/) . (a /2/)
  let lns = ((a /3/) | aa)*


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
