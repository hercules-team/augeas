module Fail_double_maybe =

  (* We can not allow this: for a string " = " we have no way to know  *)
  (* during put whether to use any of the lenses within the '?' or not *)
  (* since we key that decision off either a tree label (there is none *)
  (* here) or the presence of a value (which in this case may or may   *)
  (* not be there)                                                     *)
  let lns = (del /[ \t]*=/ "=" . (del /[ \t]*/ "" . store /[a-z]+/)? )?
