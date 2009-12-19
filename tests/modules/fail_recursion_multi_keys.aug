module Fail_recursion_multi_keys =

(* This is the same as (key "a")* . store "x" *)
let rec l = key "a" . l | store "x"
