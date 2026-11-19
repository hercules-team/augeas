module Fail_nocase_mixed_epsilon =

  (* Combining a case-insensitive regexp that matches the empty string
     with case-sensitive ones is not supported. It used to produce a
     regexp value without a regexp, and subtracting from that crashed *)
  let s = // . /m|/i - //
