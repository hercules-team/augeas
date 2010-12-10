module Pass_square =

(*  Utilities lens *)
let dels (s:string) = del s s

(************************************************************************
 *                           Regular square lens
 *************************************************************************)

(* Simplest square lens *)
let s = store /[yz]/
let sqr0 = [ square "x" s ] *
test sqr0 get "xyxxyxxyx" = { "x" = "y" }{ "x" = "y" }{ "x" = "y" }
test sqr0 put "xyx" after set "/x[3]" "z" = "xyxxzx"

(* test mismatch tag *)
test sqr0 get "xya" = *

(* Test regular expression matching with multiple groups *)
let body = del /([f]+)([f]+)/ "ff" . del /([g]+)([g]+)/ "gg"
let sqr1 = [ square /([a-b]*)([a-b]*)([a-b]*)/ body . del /([x]+)([x]+)/ "xx" ] *

test sqr1 get "aaffggaaxxbbffggbbxx" = { "aa" }{ "bb" }
test sqr1 get "affggaxx" = { "a" }
test sqr1 put "affggaxx" after clear "/b" = "affggaxxbffggbxx"

(* Test XML like elements up to depth 2 *)
let b = del ">" ">" . del /[a-z ]*/ "" . del "</" "</"
let xml = [ del "<" "<" . square /[a-z]+/ b . del ">" ">" ] *

let b2 = del ">" ">" . xml . del "</" "</"
let xml2 = [ del "<" "<" . square /[a-z]+/ b2 . del ">" ">" ] *

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

(************************************************************************
 *                        Recursive square lens
 *************************************************************************)

(* Basic element *)
let xml_element (body:lens) =
    let g = del ">" ">" . body . del "</" "</" in
        [ del "<" "<" . square /[a-z]+/ g . del ">" ">" ] *

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
let rec z = [ square "ab" z? ]
test z get "abab" = { "ab" }

(* test tip handling when using store inside body *)
let c (body:lens) =
    let sto = store "c" . body* in
        [ square "ab" sto ]

let rec cc = c cc

test cc get "abcabcabab" =
  { "ab" = "c"
    { "ab" = "c" }
  }

(* test correct put behavior *)
let input3 = "aaxyxbbaaaxyxbb"
let b3 = dels "y"
let sqr3 = [ del /[a]*/ "a" . square /[x]/ b3 . del /[b]*/ "b" ]*
test sqr3 get input3 = { "x" }{ "x" }
test sqr3 put input3 after clear "/x[1]" = input3

let b4 = del "x" "x"
let rec sqr4 = [ del /[a]+/ "a" . square /[b]|[c]/ (b4|sqr4) ]
test sqr4 put "aabaaacxcb" after rm "x" = "aabaaacxcb"
