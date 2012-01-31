(*
Module: Protocols
  Parses /etc/protocols

Author: Raphael Pinson <raphink@gmail.com>

About: Reference
  This lens tries to keep as close as possible to `man 5 protocols` where possible.

About: License
   This file is licenced under the LGPLv2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/protocols. See <filter>.

About: Examples
   The <Test_Protocols> file contains various examples and tests.
*)


module Protocols =

autoload xfm

(* View: protocol *)
let protocol =
   let alias = [ label "alias" . store Rx.word ]
   in [ key Rx.word . Sep.space . store Rx.integer
      . (Sep.space . Build.opt_list alias Sep.space)?
      . Util.comment_or_eol ]

(* View: lns
     The protocols lens *)
let lns = (Util.empty | Util.comment | protocol)*

(* Variable: filter *)
let filter = incl "/etc/protocols"

let xfm = transform lns filter
