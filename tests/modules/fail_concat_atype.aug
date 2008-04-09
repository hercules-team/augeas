module Fail_concat_atype =

  (* This passes the ctype check for unambiguous concatenation *)
  (* because the STORE's keep everything copacetic, but in the *)
  (* PUT direction, we can't tell how to split a tree          *)
  (*    { "a" = .. } { "b" = .. } { "a" = .. }                 *)
  (* solely by looking at tree labels.                         *)
  (* Ultimately, Augeas should check ful tree schemas as the   *)
  (* atype, but for now we stick to just tree labels           *)

  let ab = [ key /a/ . store /1/ ] . ([ key /b/ . store /2/ ]?)
  let ba = ([ key /b/ . store /3/ ])? . [ key /a/ . store /4/ ]
  let lns = ab . ba


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
