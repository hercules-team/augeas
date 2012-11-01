module Pass_square =

(*  Utilities lens *)
let dels (s:string) = del s s

(************************************************************************
 *                           Regular square lens
 *************************************************************************)

(* Simplest square lens *)
let s = store /[ab]/
let sqr0 =
	let k = key "x" in
	let d = dels "x" in
	[ square k s d ] *
test sqr0 get "xaxxbxxax" = { "x" = "a" }{ "x" = "b" }{ "x" = "a" }
test sqr0 put "xax" after set "/x[3]" "b" = "xaxxbx"

(* test mismatch tag *)
test sqr0 get "xya" = *

(* Test regular expression matching with multiple groups *)
let body = del /([f]+)([f]+)/ "ff" . del /([g]+)([g]+)/ "gg"
let sqr1 =
	let k = key /([a-b]*)([a-b]*)([a-b]*)/ in
	let d1 = del /([a-b]*)([a-b]*)([a-b]*)/ "a" in
	let d2 = del /([x]+)([x]+)/ "xx" in
	[ square k body d1 . d2 ] *

test sqr1 get "aaffggaaxxbbffggbbxx" = { "aa" }{ "bb" }
test sqr1 get "affggaxx" = { "a" }
test sqr1 put "affggaxx" after clear "/b" = "affggaxxbffggbxx"

(* Test XML like elements up to depth 2 *)
let b = del ">" ">" . del /[a-z ]*/ "" . del "</" "</"
let open_tag = key /[a-z]+/
let close_tag = del /[a-z]+/ "a"
let xml = [ del "<" "<" . square open_tag b close_tag . del ">" ">" ] *

let b2 = del ">" ">" . xml . del "</" "</"
let xml2 = [ del "<" "<" . square open_tag b2 close_tag . del ">" ">" ] *

test xml get "<a></a><b></b>" = { "a" }{ "b" }

(* test error on mismatch tag *)
test xml get "<a></a><b></c>" = *

(* test get nested tags of depth 2 *)
test xml2 get "<a><b></b><c></c></a>" =
  { "a"
    { "b" }
    { "c" }
  }

(* test nested put of depth 2 *)
test xml2 put "<a></a>" after clear "/x/y" = "<a></a><x><y></y></x>"

(* test nested put of depth 3 : should fail *)
test xml2 put "<a></a>" after clear "/x/y/z" = *

(* matches can be case-insensitive *)
let s5 = store /[yz]/
let sqr5 =
	let k = key /x/i in
	let d = del /x/i "x" in
	[ square k s5 d ] *
test sqr5 get "xyX" = { "x" = "y" }
test sqr5 get "xyXXyxXyx" = { "x" = "y" }{ "X" = "y" }{ "X" = "y" }
test sqr5 put "xyX" after set "/x[3]" "z" = "xyxxzx"

(* test concat multiple squares *)
let rex = /[a-z]/
let csqr =
	let k = key rex in
	let d = del rex "a" in
	let e = dels "" in
	[ square k e d . square d e d ] *

test csqr get "aabbccdd" = { "a" } { "c" }
test csqr put "aabb" after insa "z" "/a" = "aabbzzaa"

(* test default square create values *)
let create_square =
	let d = dels "a" in
	[ key "x" . square d d d ]*

test create_square put "" after clear "/x" = "xaaa"

(* test optional quotes *)
let word = /[A-Za-z0-9_.-]+/
let entry =
  let k = key word in
  let quote = del /"?/ "\"" (* " *) in
  let body = store /[a-z]+/ in
  let v = square quote body quote in
  [ k . dels "=" . v ]

test entry get "key=\"value\"" = { "key" = "value" }
test entry get "key=value" = { "key" = "value" }

test entry put "key=value" after
  set "/key" "other" = "key=other"

test entry put "key=\"value\"" after
  set "/key" "other" = "key=\"other\""

(* create with square *)
(* Passing this test successfully requires that the skeleton from the get *)
(* is correctly detected as not matching the skeleton for the second lens *)
(* in hte union - the reason for the mismatch is that the quote is        *)
(* optional in the first branch of the union, and the skeleton therefore  *)
(* does not have "@" in the right places, triggering a create             *)
let sq_create =
  let word = store /[a-z]+/ in
  let number = store /[0-9]+/ in
  let quot = dels "@" in
  let quot_opt = del /@?/ "@" in
    [ label "t" . square quot_opt word quot_opt ]
  | [ label "t" . square quot number quot ]

test sq_create put "abc" after
  set "/t" "42" = "@42@"
