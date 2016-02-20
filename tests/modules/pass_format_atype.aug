module Pass_format_atype =

(* format_atype in lens.c had an allocation bug that would corrupt memory
   and later lead to unpleasant behavior like segfaults. This nonsensical
   lens, when formatted, triggers that problem.

   For this test to make sense, it needs to be run with MALLOC_CHECK_=2 so
   that glibc complains and aborts the test
*)

let l = [label "l"]|del /x/ "x"
let _ = lens_format_atype l
