module Pass_iter_key_union =

  (* We used to typecheck the atype of this as (a|b/)*               *)
  (* which is wrong and leads to spurious ambiguous iteration errors *)
  (* The right atype is ((a|b)/)*                                    *)
  let l1 = [ key /a|b/ . store /x/ ]
  let l2 = [ key /ab/ . store /y/ ]

  let lns = (l1 | l2)*

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
