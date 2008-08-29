(* Check that we properly discard skeletons when we shift subtrees   *)
(* across lenses. The test shifts a subtree parsed with SND to one   *)
(* that is put with FST. Since the two have different skeleton types *)
(* we need to ignore the skeleton that comes with it.                *)
module Pass_array =

  let array = 
    let array_value = store /[a-z0-9]+/ in
    let fst = seq "values" . array_value . del /[ \t]+/ "\t" in
    let snd = seq "values" . array_value in
    del "(" "(" . counter "values" .
    [ fst ] * . [ snd ] . del ")" ")"

  let lns = [ key /[a-z]+/ . del "=" "=" . array ]

  test lns put "var=(v1 v2)" after
    set "var/3" "v3" = "var=(v1 v2\tv3)"
