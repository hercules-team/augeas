(* PHP module for Augeas                      *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)

module PHP =
  autoload xfm

(* PHP is a standard INI file *)

let entry  = IniFile.entry /[a-z][a-z0-9\._-]+/

let record = IniFile.record "section" entry
let lns    = IniFile.lns record

let filter = (incl "/etc/php*/*/php.ini")
             . Util.stdexcl

let xfm = transform lns filter

