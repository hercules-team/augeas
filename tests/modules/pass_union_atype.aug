(* Test that we take the right branch in a union based solely on *)
(* differing values associated with a tree node                  *)
module Pass_union_atype =
  let del_str (s:string) = del s s

  let lns = [ key /a/ . store /b/ . del_str " (l)" ]
          | [ key /a/ . store /c/ . del_str " (r)" ]

  test lns put "ac (r)" after set "a" "b" = "ab (l)"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
