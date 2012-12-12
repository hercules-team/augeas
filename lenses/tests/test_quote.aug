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

(* Group: quote_spaces *)

(* View: quote_spaces *)
let quote_spaces =
  Quote.quote_spaces (label "spc")

(* Test: quote_spaces
     Unquoted value *)
test quote_spaces get "this" =
  { "spc" = "this" }

(* Test: quote_spaces
     double quoted value *)
test quote_spaces get "\"this\"" =
  { "spc" = "this" }

(* Test: quote_spaces
     single quoted value *)
test quote_spaces get "'this'" =
  { "spc" = "this" }

(* Test: quote_spaces
     unquoted value with spaces *)
test quote_spaces get "this that" = *

(* Test: quote_spaces
     double quoted value with spaces *)
test quote_spaces get "\"this that\"" =
  { "spc" = "this that" }

(* Test: quote_spaces
     single quoted value with spaces *)
test quote_spaces get "'this that'" =
  { "spc" = "this that" }

(* Test: quote_spaces
     remove spaces from double-quoted value *)
test quote_spaces put "\"this that\""
  after set "spc" "thisthat" =
  "\"thisthat\""

(* Test: quote_spaces
     remove spaces from single-quoted value *)
test quote_spaces put "'this that'"
  after set "spc" "thisthat" =
  "'thisthat'"

(* Test: quote_spaces
     add spaces to unquoted value *)
test quote_spaces put "this"
  after set "spc" "this that" =
  "\"this that\""

(* Test: quote_spaces
     add spaces to double-quoted value *)
test quote_spaces put "\"this\""
  after set "spc" "this that" =
  "\"this that\""

(* Test: quote_spaces
     add spaces to single-quoted value *)
test quote_spaces put "'this'"
  after set "spc" "this that" =
  "'this that'"

(* Group: dquote_spaces *)

(* View: dquote_spaces *)
let dquote_spaces =
  Quote.dquote_spaces (label "spc")

(* Test: dquote_spaces
     Unquoted value *)
test dquote_spaces get "this" =
  { "spc" = "this" }

(* Test: dquote_spaces
     double quoted value *)
test dquote_spaces get "\"this\"" =
  { "spc" = "this" }

(* Test: dquote_spaces
     single quoted value *)
test dquote_spaces get "'this'" =
  { "spc" = "'this'" }

(* Test: dquote_spaces
     unquoted value with spaces *)
test dquote_spaces get "this that" = *

(* Test: dquote_spaces
     double quoted value with spaces *)
test dquote_spaces get "\"this that\"" =
  { "spc" = "this that" }

(* Test: dquote_spaces
     single quoted value with spaces *)
test dquote_spaces get "'this that'" = *

(* Test: dquote_spaces
     remove spaces from double-quoted value *)
test dquote_spaces put "\"this that\""
  after set "spc" "thisthat" =
  "\"thisthat\""

(* Test: dquote_spaces
     add spaces to unquoted value *)
test dquote_spaces put "this"
  after set "spc" "this that" =
  "\"this that\""

(* Test: dquote_spaces
     add spaces to double-quoted value *)
test dquote_spaces put "\"this\""
  after set "spc" "this that" =
  "\"this that\""

(* Test: dquote_spaces
     add spaces to single-quoted value *)
test dquote_spaces put "'this'"
  after set "spc" "this that" =
  "\"this that\""

(* Group: squote_spaces *)

(* View: squote_spaces *)
let squote_spaces =
  Quote.squote_spaces (label "spc")

(* Test: squote_spaces
     Unquoted value *)
test squote_spaces get "this" =
  { "spc" = "this" }

(* Test: squote_spaces
     double quoted value *)
test squote_spaces get "\"this\"" =
  { "spc" = "\"this\"" }

(* Test: squote_spaces
     single quoted value *)
test squote_spaces get "'this'" =
  { "spc" = "this" }

(* Test: squote_spaces
     unquoted value with spaces *)
test squote_spaces get "this that" = *

(* Test: squote_spaces
     double quoted value with spaces *)
test squote_spaces get "\"this that\"" = *

(* Test: squote_spaces
     single quoted value with spaces *)
test squote_spaces get "'this that'" =
  { "spc" = "this that" }

(* Test: squote_spaces
     remove spaces from single-quoted value *)
test squote_spaces put "'this that'"
  after set "spc" "thisthat" =
  "'thisthat'"

(* Test: squote_spaces
     add spaces to unquoted value *)
test squote_spaces put "this"
  after set "spc" "this that" =
  "'this that'"

(* Test: squote_spaces
     add spaces to double-quoted value *)
test squote_spaces put "\"this\""
  after set "spc" "this that" =
  "'this that'"

(* Test: squote_spaces
     add spaces to single-quoted value *)
test squote_spaces put "'this'"
  after set "spc" "this that" =
  "'this that'"
