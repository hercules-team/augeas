module Pass_Region =

(* Some basic lenses that we will combine in a number of ways *)
let eol = del "\n" "\n"
let indent = del /[ _]*/ "_"  (* Create spaces as '_' *)
let entry = key /a/ . del "=" "=" . store /[sx]*/

(* indent outside the subtree for entry *)
let outside = indent . [ entry ] . eol
(* indent inside the subtree for entry *)
let inside = [ indent . entry ] . eol

(* The test text is set up so that the number of 's' indicate how many
   spaces of indent we had *)
let text = "a=\n a=s\n  a=ss\n"

(* All these lenses produce the exact same tree, with or without < .. > *)
test (outside*) get text =
  { "a" = "" }
  { "a" = "s" }
  { "a" = "ss" }

test (inside*) get text =
  { "a" = "" }
  { "a" = "s" }
  { "a" = "ss" }

test (< outside >*) get text =
  { "a" = "" }
  { "a" = "s" }
  { "a" = "ss" }

test (< inside >*) get text =
  { "a" = "" }
  { "a" = "s" }
  { "a" = "ss" }

(* They behave slightly differently when we insert in the middle,
   depending on whether we use < .. > or not *)
let text_new_middle = "a=\n_a=sx\n a=s\n  a=ss\n"
let text_new_end = "a=\n a=sx\n  a=s\n_a=ss\n"

test (outside*) put text after insa "a" "/a[1]"; set "/a[2]" "sx" =
  text_new_end

test (inside*) put text after insa "a" "/a[1]"; set "/a[2]" "sx" =
  text_new_end

test (< outside >*) put text after insa "a" "/a[1]"; set "/a[2]" "sx" =
  text_new_middle

test (< inside >*) put text after insa "a" "/a[1]"; set "/a[2]" "sx" =
  text_new_middle

(* Delete an entry in the middle *)
let text_rm_shift = "a=\n a=ss\n"
let text_rm_noshift = "a=\n  a=ss\n"
test (outside*) put text after rm "/a[2]" = text_rm_shift

test (inside*) put text after  rm "/a[2]" = text_rm_shift

test (< outside >*) put text after rm "/a[2]" = text_rm_noshift

test (< inside >*) put text after rm "/a[2]" = text_rm_noshift
