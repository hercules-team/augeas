module Fail_regexp_minus_empty =

  (* This fails since the result of the subtraction is the empty set *)
  (* which we can not represent as a regular expression.             *)
  let re = "a" - /[a-z]/

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
