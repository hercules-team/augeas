module Pass_square_rec =

(*  Utilities lens *)
let dels (s:string) = del s s

(************************************************************************
 *                        Recursive square lens
 *************************************************************************)
(* test square with left and right as dels *)
let lr (body:lens) =
    let k = key "c" . body* in
    let d = dels "ab" in
        [ square d k d ]

let rec lr2 = lr lr2

test lr2 get "abcabcabab" =
  { "c"
    { "c" }
  }

let open_tag = key /[a-z]+/
let close_tag = del /[a-z]+/ "a"

(* Basic element *)
let xml_element (body:lens) =
    let g = del ">" ">" . body . del "</" "</" in
        [ del "<" "<" . square open_tag g close_tag . del ">" ">" ] *

let rec xml_rec = xml_element xml_rec

test xml_rec get "<a><b><c><d><e></e></d></c></b></a>" =
  { "a"
    { "b"
      { "c"
        { "d"
          { "e" }
        }
      }
    }
  }

test xml_rec get "<a><b></b><c></c><d></d><e></e></a>" =
  { "a"
    { "b" }
    { "c" }
    { "d" }
    { "e" }
  }

test xml_rec put "<a></a><b><c></c></b>" after clear "/x/y/z" = "<a></a><b><c></c></b><x><y><z></z></y></x>"

(* mismatch tag *)
test xml_rec get "<a></c>" = *
test xml_rec get "<a><b></b></c>" = *
test xml_rec get "<a><b></c></a>" = *


(* test ctype_nullable and typecheck *)
let rec z =
	let k = key "ab" in
	let d = dels "ab" in
	[ square k z? d ]
test z get "abab" = { "ab" }

(* test tip handling when using store inside body *)
let c (body:lens) =
    let sto = store "c" . body* in
    let d = dels "ab" in
    let k = key "ab" in
        [ square k sto d ]

let rec cc = c cc

test cc get "abcabcabab" =
  { "ab" = "c"
    { "ab" = "c" }
  }

(* test mixing regular and recursive lenses *)

let reg1 =
	let k = key "y" in
	let d = dels "y" in
	let e = dels "" in
	[ square k e d ]

let reg2 =
	let k = key "y" in
	let d = dels "y" in
	[ square k reg1 d ]

let rec rec2 =
	let d1 = dels "x" in
	let k1 = key "x" in
	let body = reg2 | rec2 in
	[ square k1 body d1 ]?

test rec2 get "xyyyyx" =
  { "x"
    { "y"
      { "y" }
    }
  }

test rec2 put "" after clear "/x/y/y" = "xyyyyx"

(* test correct put behavior *)
let input3 = "aaxyxbbaaaxyxbb"
let b3 = dels "y"
let sqr3 =
	let k = key /[x]/ in
	let d = dels "x" in
	[ del /[a]*/ "a" . square k b3 d . del /[b]*/ "b" ]*
test sqr3 get input3 = { "x" }{ "x" }
test sqr3 put input3 after clear "/x[1]" = input3

let b4 = dels "x"
let rec sqr4 =
	let k = key /[b]|[c]/ in
	let d = del /[b]|[c]/ "b" in
	[ del /[a]+/ "a" . square k (b4|sqr4) d ]
test sqr4 put "aabaaacxcb" after rm "x" = "aabaaacxcb"

(* test concat multiple squares *)
let rex = /[a-z]/
let rec csqr =
	let k = key rex in
	let d = del rex "a" in
	let e = dels "" in
	[ square k e d . csqr* . square d e d ]

test csqr get "aabbccdd" =
  { "a"
    { "b" }
  }

test csqr put "aabbccdd" after clear "/a" = "aabbccdd"
test csqr put "aabb" after clear "/a/z" = "aazzaabb"
