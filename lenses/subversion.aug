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

(* View: list_entry
     A generic entry with listed items *)
let list_entry (kw:regexp) (sto_re:regexp) (sep:lens) =
  Build.key_value_line kw Sep.space_equal
    (counter "elem" . Build.opt_list
      [ seq "elem" . store sto_re ]
      sep)

(* View: entry *)
let entry    =
     let comma_list_re = "password-stores"
  in let space_list_re = "global-ignores" | "preserved-conflict-file-exts"
  in let std_re = /[^ \t\n\/=#]+/ - comma_list_re - space_list_re
  in IniFile.entry_multiline_nocomment std_re sep comment
   | list_entry comma_list_re Rx.word Sep.comma
   | list_entry space_list_re Rx.no_spaces (del /\n?[ \t]+/ " ")



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
