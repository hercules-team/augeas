(* PHP module for Augeas                      *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)

module PHP =
  autoload xfm

(************************************************************************
 * INI File settings
 *************************************************************************)

let comment  = IniFile.comment IniFile.comment_re IniFile.comment_default
let sep      = IniFile.sep IniFile.sep_re IniFile.sep_default


(************************************************************************
 *                        ENTRY
 *
 * We have to remove the keyword "section" from possible entry keywords
 * otherwise it would lead to an ambiguity with the "section" label
 * since PHP allows entries outside of sections.
 *************************************************************************)
let entry_re = ( /[A-Za-z][A-Za-z0-9\._-]+/ - /#comment/ - /section/ )
let entry    = IniFile.entry entry_re sep comment


(************************************************************************
 *                         TITLE
 *
 * We use IniFile.title_label because there can be entries
 * outside of sections whose labels would conflict with section names
 *************************************************************************)
let title   = IniFile.title_label "section" IniFile.record_label_re
let record  = IniFile.record title entry


(************************************************************************
 *                         LENS & FILTER
 * There can be entries before any section
 * IniFile.entry includes comment management, so we just pass entry to lns
 *************************************************************************)
let lns    = IniFile.lns record entry

let filter = (incl "/etc/php*/*/php.ini")
             . Util.stdexcl

let xfm = transform lns filter

