(* Avahi module for Augeas
 Author: Athir Nuaimi <athir@nuaimi.com>

 avahi-daemon.conf is a standard INI File.
*)

module Avahi =
  autoload xfm

(************************************************************************
 * INI File settings
 *
 * avahi-daemon.conf only supports "# as commentary and "=" as separator
 *************************************************************************)
let comment    = IniFile.comment "#" "#"
let sep        = IniFile.sep "=" "="

(************************************************************************
 *                        ENTRY
 * avahi-daemon.conf uses standard INI File entries
 *************************************************************************)
let entry   = IniFile.indented_entry IniFile.entry_re sep comment

(************************************************************************
 *                        RECORD
 * avahi-daemon.conf uses standard INI File records
 *************************************************************************)
let title   = IniFile.indented_title IniFile.record_re
let record  = IniFile.record title entry

(************************************************************************
 *                        LENS & FILTER
 * avahi-daemon.conf uses standard INI File records
 *************************************************************************)
let lns     = IniFile.lns record comment

let filter = (incl "/etc/avahi/avahi-daemon.conf")

let xfm = transform lns filter
