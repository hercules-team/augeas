module Fail_nocase_mixed_union =

  (* See fail_nocase_mixed_epsilon *)
  let r = (/a/ | /m|/i)*
  let s = r - /a/
