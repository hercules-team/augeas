module Pass_quote_quote =

  let str = "\"A quote\""

  let delq = del /"/ "\""

  let lns = [ delq . key /[a-zA-Z ]+/ . delq ] (* " Make emacs relax *)

  test lns get str = { "A quote" }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
