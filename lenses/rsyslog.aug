(*
Module: Rsyslog
  Parses /etc/rsyslog.conf

Author: Raphael Pinson <raphael.pinsons@camptocamp.com>

About: Reference
  This lens tries to keep as close as possible to `man 5 rsyslog.conf` where possible.

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/rsyslog.conf. See <filter>.

About: Examples
   The <Test_Rsyslog> file contains various examples and tests.
*)
module Rsyslog =

autoload xfm

let macro = [ key /$[A-Za-z0-9]+/ . Sep.space . store Rx.neg1 . Util.comment_or_eol ]

let entries = ( Syslog.empty | Syslog.comment | Syslog.entry | macro )*

let lns = entries . ( Syslog.program | Syslog.hostname )*

let filter = incl "/etc/rsyslog.conf"

let xfm = transform lns filter

