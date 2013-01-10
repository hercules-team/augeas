(*
Module: Subversion
  Parses subversion's INI files

Authors:
   Marc Fournier <marc.fournier@camptocamp.com>
   Raphael Pinson <raphael.pinson@camptocamp.com>

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Examples
   The <Test_Subversion> file contains various examples and tests.

*)

module Subversion =
autoload xfm

(************************************************************************
 * Group: INI File settings
 *
 * subversion only supports comments starting with "#"
 *
 *************************************************************************)

(* View: comment *)
let comment  = IniFile.comment_noindent "#" "#"

(* View: sep *)
let sep      = IniFile.sep IniFile.sep_default IniFile.sep_default

(************************************************************************
 * Group:                  ENTRY
 *
 * subversion doesn't support indented entries
 *
 *************************************************************************)

(* Variable: comma_list_re *)
let comma_list_re = "password-stores"

(* Variable: space_list_re *)
let space_list_re = "global-ignores" | "preserved-conflict-file-exts"

(* Variable: std_re *)
let std_re = /[^ \t\n\/=#]+/ - (comma_list_re | space_list_re)

(* View: entry_std
    A standard entry
    Similar to a <IniFile.entry_multiline_nocomment> entry,
    but allows ';' *)
let entry_std =
     let newline = /\n[ \t]+/
  in let bare =
          let base_re = /[^#" \t\n]+([ \t]+[^#" \t\n]+)*/
       in Quote.do_dquote_opt_nil (store (base_re . (newline . base_re)*))
  in let quoted =
          let base_re = /[^"\n]*[#]+[^"\n]*/
       in Quote.do_dquote (store (base_re . (newline . base_re)*))
  in [ key std_re . sep . (Sep.opt_space . bare)? . IniFile.eol ]
   | [ key std_re . sep . Sep.opt_space . quoted . IniFile.eol ]
   | comment

(* View: entry *)
let entry    =
     let comma_list_re = "password-stores"
  in let space_list_re = "global-ignores" | "preserved-conflict-file-exts"
  in let std_re = /[^ \t\n\/=#]+/ - (comma_list_re | space_list_re)
  in entry_std
   | IniFile.entry_list_nocomment comma_list_re sep Rx.word Sep.comma
   | IniFile.entry_list_nocomment space_list_re sep Rx.no_spaces (del /\n?[ \t]+/ " ")



(************************************************************************
 * Group:                    TITLE
 *
 * subversion doesn't allow anonymous entries (outside sections)
 *
 *************************************************************************)

(* View: title *)
let title    = IniFile.title IniFile.entry_re

(* View: record *)
let record   = IniFile.record title entry

(************************************************************************
 * Group:                   LENS & FILTER
 *************************************************************************)

(* View: lns *)
let lns      = IniFile.lns record comment

(* Variable: filter *)
let filter   = incl "/etc/subversion/config"
             . incl "/etc/subversion/servers"

let xfm      = transform lns filter
