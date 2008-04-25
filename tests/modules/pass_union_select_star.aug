module Pass_union_select_star =

  (* Check that in unions the right branch is selected, even if the *)
  (* first branch matches nothing.                                  *)
  let a = [ key /a/ ]
  let b = [ key /b/ ]

  let lns = (a* | b+)

  (* The 'rm "x"' is a noop; the grammar won't let us do a put test *)
  (* without any commands                                           *)
  test lns put "b" after rm "x" = "b"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
