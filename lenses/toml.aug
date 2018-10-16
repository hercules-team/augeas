(*
Module: Toml
  Parses TOML files

Author: Raphael Pinson <raphael.pinson@camptocamp.com>

About: Reference
  https://github.com/mojombo/toml/blob/master/README.md

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to TOML files.

About: Examples
   The <Test_Toml> file contains various examples and tests.
*)

module Toml =

(* Group: base definitions *)

(* View: sep
     The key/value separator *)
let sep = IniFile.sep "=" "="

(* View: comment
     A simple comment *)
let comment  = IniFile.comment "#" "#"

(* View: empty
     An empty line *)
let empty = Util.empty

(* View: eol
     An end of line *)
let eol = Util.eol


(* Group: value entries *)

let bare_re_noquot = (/[^][", \t\r\n]/ - "#")
let bare_re = (/[^][,\r=]/ - "#")+
let no_quot = /[^]["\r\n]*/
let bare = Quote.do_dquote_opt_nil (store (bare_re_noquot . (bare_re* . bare_re_noquot)?))
let quoted = Quote.do_dquote (store (no_quot . "#"+ . no_quot))

(* View: array
     A (potentially recursive) array of values *)
let rec array =
     (* Not using seq to avoid counter mess up with reentrancy *)
     let elem = [ label "elem" . bare ]
              | [ label "elem" . quoted ]
              | [ label "elem" . array ]
  in let comma = del /[ \t\n]*,[ \t\n]*/ ","
  in del /\[[ \t\n]*/ "["
   . Build.opt_list elem comma
   . del /[ \t\n]*\]/ "]"

(* View: entry_gen
     A generic entry *)
let entry_gen (sto:lens) = [ Util.indent . key IniFile.entry_re
                           . sep . Sep.opt_space . sto . (comment|eol) ]

(* View: entry
     An entry *)
let entry = entry_gen bare
          | entry_gen quoted
          | entry_gen array

(* Group: tables *)

(* View: table_gen
     A generic table *)
let table_gen (name:string) (lbrack:string) (rbrack:string) =
     let title = Util.indent . label name
               . Util.del_str lbrack
               . store /[^]\r\n.]+(\.[^]\r\n.]+)*/
               . Util.del_str rbrack . eol
  in [ title . (entry|empty|comment)* ]

(* View: table
     A table or array of tables *)
let table = table_gen "@table" "[" "]"
          | table_gen "@@table" "[[" "]]"

(* Group: lens *)

(* View: lns
     The Toml lens *)
let lns = (entry | empty | comment)* . table*
