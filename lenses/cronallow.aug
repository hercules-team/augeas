(*
Module: CronAllow
  Parses /etc/{at,cron}.{allow,deny}

Author: Dominic Cleal <dcleal@redhat.com>

About: Reference
  This lens follows `man 1 crontab` and the cronie source code.

About: Licence
  This file is licensed under the LGPLv2+, like the rest of Augeas.

About: Lens Usage

About: Configuration files
  This lens applies to /etc/{at,cron}.{allow,deny}.conf and the same under
  /etc/cron.d for Solaris. See <filter>.
*)

module CronAllow =
autoload xfm

(************************************************************************
 * Group:                 USEFUL PRIMITIVES
 *************************************************************************)

(* View: comment *)
let comment = Util.comment

(* View: empty *)
let empty = Util.empty

(* View: user
    A single line containing a username *)
let user = [ seq "user" . store Rx.word . Util.eol ]

(* View: lns *)
let lns = ( empty | comment | user )*

(* Variable: filter *)
let filter = incl "/etc/at.allow"
           . incl "/etc/at.deny"
           . incl "/etc/cron.allow"
           . incl "/etc/cron.deny"
           . incl "/etc/cron.d/at.allow"
           . incl "/etc/cron.d/at.deny"
           . incl "/etc/cron.d/cron.allow"
           . incl "/etc/cron.d/cron.deny"
           . Util.stdexcl

let xfm = transform lns filter
