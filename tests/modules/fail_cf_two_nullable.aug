module Fail_cf_two_nullable =

(* Both branches of the union are nullable, making SPC ambiguous *)
let rec spc = [ spc . label "x" ] | del /s*/ "s"
