(*
Module: Test_Quote
  Provides unit tests and examples for the <Quote> lens.
*)

module Test_Quote =

(* View: double *)
let double = [ label "double" . Quote.double ]

(* Test: double *)
test double get "\" this is a test\"" =
  { "double" = " this is a test" }

(* View: double_opt *)
let double_opt = [ label "double_opt" . Quote.double_opt ]

(* Test: double_opt *)
test double_opt get "\"this is a test\"" =
  { "double_opt" = "this is a test" }

(* Test: double_opt *)
test double_opt get "this is a test" =
  { "double_opt" = "this is a test" }

(* Test: double_opt
   Value cannot start with a space *)
test double_opt get " this is a test" = *

(* View: single *)
let single = [ label "single" . Quote.single ]

(* Test: single *)
test single get "' this is a test'" =
  { "single" = " this is a test" }

(* View: single_opt *)
let single_opt = [ label "single_opt" . Quote.single_opt ]

(* Test: single_opt *)
test single_opt get "'this is a test'" =
  { "single_opt" = "this is a test" }

(* Test: single_opt *)
test single_opt get "this is a test" =
  { "single_opt" = "this is a test" }

(* Test: single_opt
   Value cannot start with a space *)
test single_opt get " this is a test" = *

(* View: any *)
let any = [ label "any" . Quote.any ]

(* Test: any *)
test any get "\" this is a test\"" =
  { "any" = " this is a test" }

(* Test: any *)
test any get "' this is a test'" =
  { "any" = " this is a test" }

(* View: any_opt *)
let any_opt = [ label "any_opt" . Quote.any_opt ]

(* Test: any_opt *)
test any_opt get "\"this is a test\"" =
  { "any_opt" = "this is a test" }

(* Test: any_opt *)
test any_opt get "'this is a test'" =
  { "any_opt" = "this is a test" }

(* Test: any_opt *)
test any_opt get "this is a test" =
  { "any_opt" = "this is a test" }

(* Test: any_opt
   Value cannot start with a space *)
test any_opt get " this is a test" = *

(* View: double_opt_allow_spc *)
let double_opt_allow_spc =
  let body = store /[^\n"]+/ in
  [ label "double" . Quote.do_dquote_opt body ]

(* Test: double_opt_allow_spc *)
test double_opt_allow_spc get " test with spaces " =
  { "double" = " test with spaces " }
