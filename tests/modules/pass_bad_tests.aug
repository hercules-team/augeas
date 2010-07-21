(* Make sure that nonsensical tests fail properly instead of *)
(* causing a segfault (see bug #129)                         *)
module Pass_bad_tests =

 let k = [ key "a" ]*

 test k put "aaa" after insb "a" "/a[4]" = *
 test k put "aaa" after insa "a" "/a[4]" = *

 test k put "aa" after set "x[func()]" "foo" = *

 (* Make sure we reset internal error flags after the above *)
 test k get "aa" = { "a" } { "a" }

 test k put "aa" after rm "/a[func()]" = *

 test k put "aa" after clear "/a[func()]" = *
