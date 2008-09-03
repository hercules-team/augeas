(* The construct (del RE STR)? must raise an error from the typechecker *)
(* since there's no way for the put of '?' to determine whether the del *)
(* should be used or not - that decision is made by looking at the tree *)
(* alone.                                                               *)
module Fail_del_maybe =

let indent = (del /[ \t]+/ " ")?
