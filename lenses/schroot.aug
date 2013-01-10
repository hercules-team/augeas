(*
Module: Schroot
  Parses /etc/schroot/schroot.conf

Author: Raphael Pinson <raphink@gmail.com>

About: Reference
  This lens tries to keep as close as possible to `man 5 schroot.conf` where possible.

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/schroot/schroot.conf. See <filter>.
*)


module Schroot =
autoload xfm


(************************************************************************
 * Group:                 USEFUL PRIMITIVES
 *************************************************************************)

(* View: comment
   An <IniFile.comment> entry *)
let comment    = IniFile.comment "#" "#"

(* View: sep
   An <IniFile.sep> entry *)
let sep        = IniFile.sep "=" "="


(************************************************************************
 * Group:                 ENTRIES
 *************************************************************************)

(* View: description
   Descriptions are special entries, which can have an optional lang parameter *)
let description =
     let lang = [ Util.del_str "[" . label "lang"
                . store IniFile.entry_re . Util.del_str "]" ]
  in let bare = Quote.do_dquote_opt_nil (store /[^#;" \t\n]+([ \t]+[^#;" \t\n]+)*/)
  in let quoted = Quote.do_dquote (store /[^"\n]*[#;]+[^"\n]*/)
  in [ key "description" . lang? . sep . (Sep.opt_space . bare)? . (comment|IniFile.eol) ]
   | [ key "description" . lang? . sep . Sep.opt_space . quoted . (comment|IniFile.eol) ]

(* View: entry
   An <IniFile.entry>, or <description> *)
let entry   = IniFile.entry (IniFile.entry_re - "description") sep comment
            | description

(* View: title
   An <IniFile.title> *)
let title   = IniFile.title IniFile.record_re

(* View: record
   An <IniFile.record> *)
let record = IniFile.record title entry

(* View: lns
   An <IniFile.lns> *)
let lns    = IniFile.lns record comment

(* View: filter *)
let filter = (incl "/etc/schroot/schroot.conf")

let xfm = transform lns filter
