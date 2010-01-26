module Pass_Mixed_Recursion =

(* Test that using a recursive lens as part of another lens works *)

let rec r = [ key "a" . r ] | [ key "a" . store "x" ]

(* Star a recursive lens *)
let star = r*

test star get "aax" = { "a" { "a" = "x" } }
test star put "aax" after rm "/nothing" = "aax"

test star get "axax" = { "a" = "x" } { "a" = "x" }
test star put "axaaxax" after rm "/a[2]" = "axax"

(* Use a starred recursive lens in a more complicated construct *)
let top = [ label "top" . r* . value "high" ]

test top get "axaax" = { "top" = "high" { "a" = "x" } { "a" { "a" = "x" } } }
test top put "axaax" after rm "/top/a[1]" = "aax"

(* Use a recursive lens in a union *)
let union = (r | [ key "b" . store /[a-z]/ ])*

test union get "aaxbyax" =
  { "a" { "a" = "x" } } { "b" = "y" } { "a" = "x" }
test union put "aaxbyax" after
  set "/b[2]" "z" = "aaxbyaxbz"
test union put "aaxbyax" after
  set "/b[2]" "z"; rm "/b[1]" = "aaxaxbz"
