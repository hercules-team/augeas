module Pass_lens_plus =

  let a = [ key /a/ ]
  let lns = a+

  test lns put "a" after rm "x" = "a"


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
