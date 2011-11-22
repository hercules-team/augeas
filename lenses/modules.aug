(*
Module: Modules
  Parses /etc/modules

About: Reference
  This lens tries to keep as close as possible to `man 5 modules` where possible.

About: License
   This file is licenced under the LGPLv2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/modules. See <filter>.
*)
module Modules =
autoload xfm

(* View: word *)
let word = /[^#, \n\t\/]+/

(* View: record *)
let record = [ key word . (Util.del_ws_tab . Modprobe.sto_to_eol)? . Util.eol ]

(* View: lns *)
let lns = ( Util.empty | Util.comment | record ) *

(* View: filter *)
let filter = incl "/etc/modules"

let xfm = transform lns filter
