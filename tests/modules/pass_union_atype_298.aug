module Pass_union_atype_298 =
  (* The key calculation in union would cause the outer subtree to have  *)
  (* an atype of %r{a/(/|/)}, which is wrong since the two subtrees in   *)
  (* the union do not contribute to the key of the outer subtree         *)
  (* The tree produced by this lens has schema                           *)
  (*   { /a/ ({ = /b/ }|{}) }                                            *)
  (* which is prefectly legal                                            *)
  let lns = [ key /a/ . ([ store /b/] | del /c/ "c") ]

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
