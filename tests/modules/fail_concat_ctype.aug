module Fail_concat_ctype =

  (* Concatenation of /a|ab/ and /a|ba/ is ambiguous *)
  let lns = store /a|ab/ . (del /a/ "x" | del /ba/ "y")

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
