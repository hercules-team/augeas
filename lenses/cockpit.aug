(*
Module: Cockpit
 Cockpit module for Augeas

 Author: Pat Riehecky <riehecky@fnal.gov>

About: Configuration files
 cockpit.conf is a standard INI File.

About: License
  This file is licenced under the LGPL v2+, like the rest of Augeas.
*)

module Cockpit =
  autoload xfm

(************************************************************************
 * INI File settings
 *************************************************************************)

let comment  = IniFile.comment "#" "#"
let sep      = IniFile.sep "=" "="
let empty    = Util.empty
let eol      = IniFile.eol

(************************************************************************
 *                        ENTRY
 *************************************************************************)

let list_entry (list_key:string)  =
  let list_value = store /[^# \t\r\n,][^ \t\r\n,]*[^# \t\r\n,]|[^# \t\r\n,]/ in
  let list_sep = del /([ \t]*(,[ \t]*|\r?\n[ \t]+))|[ \t]+/ " " in
  [ key list_key . sep . Sep.opt_space . list_value ]
  . (list_sep . Build.opt_list [ label list_key . list_value ] list_sep)?
  . eol

let entry_re = IniFile.entry_re - ("Origins" | "Fatal")

let entry       = IniFile.entry entry_re sep comment
                | empty

let entries =
     let list_entry_elem (k:string) = list_entry k . entry*
  in entry*
   | entry* . Build.combine_two_opt
                (list_entry_elem "Origins")
                (list_entry_elem "Fatal")


(***********************************************************************a
 *                         TITLE
 *************************************************************************)
let title       = IniFile.title IniFile.record_re
let record      = [ title . entries ]


(************************************************************************
 *                         LENS & FILTER
 *************************************************************************)
let lns    = (empty | comment)* . record*

let filter = (incl "/etc/cockpit/cockpit.conf")

let xfm = transform lns filter
