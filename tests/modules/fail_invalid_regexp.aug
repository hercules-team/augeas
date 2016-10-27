module Fail_Invalid_Regexp =

  (* We used to not spot that the second regexp in the union is invalid
     because we did not properly check expressions that contained
     literals. This construct now leads to a syntax error *)
  let rx = /a/ | /d)/
