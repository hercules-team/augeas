module Fail_union_atype =
  (* This is illegal, otherwise we don't know which alternative *)
  (* to take for a tree { "a" = "?" }                           *)

  let del_str (s:string) = del s s

  let lns = [ key /a/ . store /b/ . del_str " (l)"  
          | [ key /a/ . store /c/ . del_str " (r)" ]

  (* To make this a passing test, make sure that this also works: *)
  (* test lns put "ac (r)" after set "a" "b" = "ab (l)"           *)

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
