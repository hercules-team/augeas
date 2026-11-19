module Fail_compose_type =

  (* Only functions and unit can be on the left of ';'. Reporting that
     used to look at the type of the right operand before it had been
     computed *)
  let x = "a" ; "b"
