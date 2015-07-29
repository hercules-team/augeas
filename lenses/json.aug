module Json =

(* A generic lens for Json files                                           *)
(* Based on the following grammar from http://www.json.org/                *)
(* Object ::= '{'Members ? '}'                                             *)
(* Members ::= Pair+                                                       *)
(* Pair ::= String ':' Value                                               *)
(* Array ::= '[' Elements ']'                                              *)
(* Elements ::= Value ( "," Value )*                                       *)
(* Value ::= String | Number | Object | Array | "true" | "false" | "null"  *)
(* String ::= "\"" Char* "\""                                              *)
(* Number ::= /-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?/                      *)

let spc = /[ \t\n]*/

let comment = Util.comment_generic /\/\/[ \t]*/ "// "
let comment_empty = [ del /\/\/[ \t]*\n/ "//\n" ]
(*
let comment_spc = comment . del spc "" | comment_empty
*)
let comment_delim = Util.empty_c_style | Util.comment_c_style | Util.comment_multiline
let comment_multiline = comment | comment_empty | Util.comment_multiline
let comment_val = del /[ \t]/ " " . (Util.comment_c_style | Util.comment_multiline)

let ws = del spc ""
let eol = del spc "\n"
let delim (c:string) (d:string) = del c d
let dels (s:string) = del s s

let comma = delim "," "," . comment_delim* . Sep.opt_space 
let colon = delim ":" ":" . comment_delim* . Sep.opt_space
let lbrace = delim "{" "{" . comment_delim* . Sep.opt_space
let rbrace = delim "}" "}"
let lbrack = delim "[" "[" . comment_delim* . Sep.opt_space
let rbrack = delim "]" "]"

let str_store =
  let q =  del "\"" "\"" in
  q . store /[^"]*/ . q             (* " Emacs, relax *)

let number = [ label "number" . store /-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?/ . comment_val* . ws ]
let str = [ label "string" . str_store . comment_delim* . Sep.opt_space ]

let const (r:regexp) = [ label "const" . store r . comment_val* . ws ]

let value0 = str | number | const /true|false|null/

let fix_value (value:lens) =
  let array = [ label "array" . lbrack . ((Build.opt_list value comma . rbrack . comment_delim* . Sep.opt_space) | (rbrack . ws)) ] in
  let pair = [ label "entry" . str_store . ws . colon . value ] in
  let obj = [ label "dict" . lbrace . ((Build.opt_list pair comma. rbrace . comment_delim* . Sep.opt_space) | (rbrace . ws)) ] in
  (str | number | obj | array | const /true|false|null/)

(* Typecheck finitely deep nesting *)
(*
let value1 = fix_value value0
let value2 = fix_value value1
*)

(* Process arbitrarily deeply nested JSON objects *)
let rec rlns = fix_value rlns

let lns = comment_multiline* . ws . rlns
