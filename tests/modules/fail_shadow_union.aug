module Fail_shadow_union =
  let lns = store /[a-z]+/ | del /(abc|xyz)/ "abc"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
