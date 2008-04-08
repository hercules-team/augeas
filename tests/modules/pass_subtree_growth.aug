module Pass_subtree_growth =

  (* Make sure that a subtree that is not the lowest one does indeed *)
  (* grow the tree, instead of just setting the label of an enclosed *)
  (* subtree. This is only a problem if the enclosed subtree does    *)
  (* not have a label                                                *)

  let lns = [ label "outer" . [ store /a/ ] ]

  (* The improper result is { "outer" = "a" } *)
  test lns get "a" = { "outer" { = "a" } }

  test lns put "a" after set "outer" "b" = "b"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
