(*
Module: Shells
  Parses /etc/shells

Author: Raphael Pinson <raphink@gmail.com>

About: Reference
  This lens tries to keep as close as possible to `man 5 shells` where possible.

About: License
   This file is licenced under the LGPLv2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/shells. See <filter>.
*)


module Shells =
  autoload xfm

let empty = Util.empty
let comment = Util.comment
let shell = [ seq "shell" . store /[^# \t\n]+/ . Util.eol ]

(* View: lns
     The shells lens
*)
let lns = ( empty | comment | shell )*

(* Variable: filter *)
let filter = incl "/etc/shells"
    . Util.stdexcl

let xfm = transform lns filter
