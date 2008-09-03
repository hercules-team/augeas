(* Test that the '?' operator behaves properly when it contains only   *)
(* a store. For this to work, the put for '?' has to take into account *)
(* whether the current tree node has a value associated with it or not *)
module Pass_store_maybe =

  let lns = [ key /[a-z]+/ . del / +/ " " . (store /[0-9]+/) ? ]

  test lns put "key " after rm "noop" = "key "
  test lns put "key " after set "key" "42" = "key 42"
  test lns put "key 42" after set "key" "13" = "key 13"

  (* Convoluted way to make the value for "key" NULL *)
  test lns put "key 42" after insa "key" "key"; rm "key[1]" = "key "

  (* Check that we correctly restore the DEL if we choose to go into the *)
  (* '?' operator instead of doing a create and using the "--" default   *)
  let ds = [ key /[a-z]+/ . (del /[ -]*/ "--" . store /[0-9]+/) ? ]

  test ds put "key 0" after rm "noop" = "key 0"
