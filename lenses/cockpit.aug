(*
Module: Cockpit
 Cockpit module for Augeas

 Author: Pat Riehecky <riehecky@fnal.gov>

 cockpit.conf is a standard INI File.
*)

module Cockpit =
  autoload xfm

(************************************************************************
 * Group: INI File settings
 *************************************************************************)
(* View: comment *)
let comment    = IniFile.comment "#" "#"
(* View: sep *)
let sep        = IniFile.sep "=" "="
(* View: empty *)
let empty       = Util.empty
(* View: eol *)
let eol        = IniFile.eol

(************************************************************************
 * Group: Entry
 *************************************************************************)
(* View: list_entry *)
let list_entry (list_key:string)  =
  let list_value = store /[^# \t\r\n,][^ \t\r\n,]*[^# \t\r\n,]|[^# \t\r\n,]/ in
  let list_sep = del /([ \t]*([ \t]*|\r?\n[ \t]+))|[ \t]+/ "\n\t" in
  [ key list_key . sep . Sep.opt_space . list_value ]
  . (list_sep . Build.opt_list [ label list_key . list_value ] list_sep)?
  . eol

let entry_re = IniFile.entry_re - ("Origins")

(* View: entry *)
let entry   = IniFile.entry entry_re sep comment | empty

let entries =
     let list_entry_elem (k:string) = list_entry k . entry*
  in entry*
   | entry* .  list_entry_elem "Origins"

(************************************************************************
 * Group: Record
 *************************************************************************)
(* View: title *)
let title   = IniFile.indented_title IniFile.record_re
(* View: record *)
let record  = IniFile.record title entries

(************************************************************************
 * Group: Lens and filter
 *************************************************************************)
(* View: lns *)
let lns     = IniFile.lns record comment

(* View: filter *)
let filter = (incl "/etc/cockpit/cockpit.conf")

let xfm = transform lns filter
