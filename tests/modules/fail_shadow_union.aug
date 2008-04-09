module Fail_shadow_union =
  let lns = store /[a-z]+/ | del /(abc|xyz)/ "never used"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
