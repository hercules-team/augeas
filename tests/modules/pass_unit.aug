module Pass_Unit =

(* The unit literal *)
let _ = ()
(* Check that composition allows units on the left *)
let _ = () ; () ; (); "something"
