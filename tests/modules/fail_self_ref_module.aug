module Fail_self_ref_module =

  let b = Fail_self_ref_module.a
  let a = "a"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
