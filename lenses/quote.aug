(*
Module: Quote
  Generic module providing useful primitives for quoting

Author: Raphael Pinson <raphink@gmail.com>

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Examples
   The <Test_Quote> file contains various examples and tests.
*)

module Quote =

(* Group: QUOTE SEPARATORS *)

(* Variable: dquote
     A double quote *)
let dquote = Util.del_str "\""

(* Variable: dquote_opt
     An optional double quote *)
let dquote_opt = del /"?/ "\""

(* Variable: squote
     A single quote *)
let squote = Util.del_str "'"

(* Variable: squote_opt
     An optional single quote *)
let squote_opt = del /'?/ "'"

(* Variable: quote
     A quote, whether double or single *)
let quote = del /["']/ "\""

(* Variable: quote
     An optional quote, whether double or single *)
let quote_opt = del /["']?/ "\""


(* Group: QUOTING FUNCTIONS *)

(* View: do_dquote *)
let do_dquote (body:lens) =
  square dquote body dquote

(* View: do_dquote_opt *)
let do_dquote_opt (body:lens) =
  square dquote_opt body dquote_opt

(* View: do_squote *)
let do_squote (body:lens) =
  square squote body squote

(* View: do_squote_opt *)
let do_squote_opt (body:lens) =
  square squote_opt body squote_opt

(* View: do_quote *)
let do_quote (body:lens) =
  square quote body quote

(* View: do_quote_opt *)
let do_quote_opt (body:lens) =
  square quote_opt body quote_opt


(* Group: QUOTED VALUES *)

(* View: double
     A double-quoted value *)
let double =
     let body = store /[^\n]*/
  in do_dquote body

(* View: double_opt
     An optionaly double-quoted value
     Double quotes are not allowed in value
     Value cannot begin or end with spaces *)
let double_opt =
     let body = store /[^\n\t "]([^\n"]*[^\n\t "])?/
  in do_dquote_opt body

(* View: single
     A single-quoted value *)
let single =
     let body = store /[^\n]*/
  in do_squote body

(* View: single_opt
     An optionaly single-quoted value
     Single quotes are not allowed in value
     Value cannot begin or end with spaces *)
let single_opt =
     let body = store /[^\n\t ']([^\n']*[^\n\t '])?/
  in do_squote_opt body

(* View: any
     A quoted value *)
let any =
     let body = store /[^\n]*/
  in do_quote body

(* View: any_opt
     An optionaly quoted value
     Double or single quotes are not allowed in value
     Value cannot begin or end with spaces *)
let any_opt =
     let body = store /[^\n\t "']([^\n"']*[^\n\t "'])?/
  in do_quote_opt body

(* View: quote_spaces
     Make quotes mandatory if value contains spaces,
     and optional if value doesn't contain spaces. *)
let quote_spaces (lns:lens) =
     (* bare has no spaces, and is optionally quoted *)
     let bare = Quote.do_quote_opt (store /[^"' \t\n]+/)
     (* quoted has at least one space, and must be quoted *)
  in let quoted = Quote.do_quote (store /[^"' \t\n]*[ \t][^"' \t\n]*/)
  in [ lns . bare ] | [ lns . quoted ]

