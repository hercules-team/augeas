module Pass_Create_Del =

(* del, on create, would do another round of unescaping the default value
 * which is wrong. See Issue #507 *)
let sep = del /:([ \t]*\\\\\n[ \t]*:)?/ ":\\\n\t:"

let lns = [ label "entry" . sep . store /[a-z]+/ ]*

test lns get ":a:\\\n:b" =
  { "entry" = "a" }
  { "entry" = "b" }

test lns put ":a" after
  set "/entry[last()+1]" "b" = ":a:\\\n\t:b"
