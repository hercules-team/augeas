module Fail_xform_orphan_value =

  let lns = store /a/

  let xfm = transform lns (incl "/dev/null")


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
