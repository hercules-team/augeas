(* Test let expressions *)
module Pass_let_exp =

  (* This definition is insanely roundabout; it's written that way *)
  (* since we want to exercise LET expressions                     *)
  let lns =
    let lbl = "a" in
    let spc = " " in
    let del_spaces (s:string) = del spc+ s in
    let del_str (s:string) = del s s in
    let store_delim (ldelim:string)
                    (rdelim:string) (val:regexp) =
      del_str ldelim . store val . del_str rdelim in
    [ label lbl . del_spaces " " . store_delim "(" ")" /[a-z]+/ ]

  test lns get "  (abc)" = { "a" = "abc" }


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
