(*
Module: ClamAV
  Parses ClamAV clamd and freshclam configuration files.

Author: Andrew Colin Kissa <andrew@topdog.za.net>
  Baruwa Enterprise Edition http://www.baruwa.com

About: License
  This file is licensed under the LGPL v2+.

About: Configuration files
  This lens applies to /etc/clamd.conf, /etc/freshclam.conf and files in
  /etc/clamd.d. See <filter>.
*)

module Clamav =
autoload xfm

(************************************************************************
 * Group:                 USEFUL PRIMITIVES
 ************************************************************************)

let word = /[A-Za-z][A-Za-z0-9]+/

let comment  = Util.comment

let some_value = Sep.space . store Rx.space_in

(************************************************************************
 * Group: Entry
 ************************************************************************)
 
let clamd_entry (kw:regexp) (comment:lens) =
                [ key "Example" . Util.eol ]
                | [ key kw . some_value . Util.eol ]
                | comment

let entry   = clamd_entry word comment

(******************************************************************
 * Group:                   LENS AND FILTER
 ******************************************************************)

(************************************************************************
 * View: Lns
 ************************************************************************)

let lns = (Util.empty | entry )*

(* Variable: filter *)
let filter = ((incl "/etc/clamd.conf")
            . (incl "/etc/freshclam.conf")
            . (incl "/etc/clamd.d/*.conf"))

let xfm = transform lns filter
