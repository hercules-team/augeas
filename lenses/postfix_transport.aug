(*
Module: Postfix_Transport
  Parses /etc/postfix/transport

Author: Raphael Pinson <raphael.pinson@camptocamp.com>

About: Reference
  This lens tries to keep as close as possible to `man 5 transport` where possible.

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/postfix/transport. See <filter>.

About: Examples
   The <Test_Transport> file contains various examples and tests.
*)

module Postfix_Transport =

autoload xfm

(* View: space_or_eol *)
let space_or_eol = del /([ \t]*\n)?[ \t]+/ " "

(* View: colon *)
let colon = Sep.colon

(* View: transport *)
let transport = [ label "transport" . (store Rx.word)? ]

(* View: nexthop *)
let nexthop = [ label "nexthop" . (store Rx.space_in)? ]

(* View: record *)
let record = [ label "pattern" . store /[A-Za-z0-9@\*.-]+/
             . space_or_eol . transport
             . colon . nexthop
             . Util.eol ]

(* View: lns *)
let lns = (Util.empty | Util.comment | record)*

(* Variable: filter *)
let filter = incl "/etc/postfix/transport"
           . incl "/etc/postfix/virtual"

let xfm = transform lns filter
