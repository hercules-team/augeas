module Fail_load_cycle_a =

  (* Fail_load_cycle_b refers back to this module; loading the two used
     to recurse until the stack overflowed *)
  let a = Fail_load_cycle_b.b
