(*
Module: Hostname
  Parses /etc/hostname

Author: Raphael Pinson <raphink@gmail.com>

About: License
   This file is licenced under the LGPLv2+, like the rest of Augeas.
*)


module Hostname =
autoload xfm

(* View: lns *)
let lns = [ label "hostname" . store Rx.word . Util.eol ]

(* View: filter *)
let filter = incl "/etc/hostname"

let xfm = transform lns filter
