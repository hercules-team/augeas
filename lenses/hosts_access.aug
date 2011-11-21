(*
Module: Hosts_Access
  Parses /etc/hosts.{allow,deny}

Author: Raphael Pinson <raphink@gmail.com>

About: Reference
  This lens tries to keep as close as possible to `man 5 hosts_access` where possible.

About: License
   This file is licenced under the LGPLv2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/hosts.{allow,deny}. See <filter>.
*)

module Hosts_Access =

autoload xfm

(************************************************************************
 * Group:                 USEFUL PRIMITIVES
 *************************************************************************)

(* View: colon *)
let colon = del /[ \t]*:[ \t]*/ ": "

(* View: comma *)
let comma = del /([ \t]|(\\\\\n))*,([ \t]|(\\\\\n))*/ ", "

(* View: sto_to_eol *)
let sto_to_eol = store /[^ \t\n:][^\n]*[^ \t\n]|[^ \t\n:]/

(* View: except
 * The except operator makes it possible to write very compact rules.
 *)
let except (lns:lens) = [ label "except" . Sep.space
                        . del /except/i "EXCEPT"
                        . Sep.space . lns ]

(************************************************************************
 * Group:                 ENTRY TYPES
 *************************************************************************)

(* View: daemon *)
let daemon =
  let host = [ label "host"
             . Util.del_str "@"
             . store Rx.word ] in
   [ label "process"
   . store Rx.word
   . host? ]

(* View: daemon_list
    A list of <daemon>s *)
let daemon_list = Build.opt_list daemon comma

(* View: client *)
let client =
  let user = [ label "user"
             . store Rx.word
             . Util.del_str "@" ] in
    [ label "client"
    . user?
    . store Rx.word ]

(* View: client_list
    A list of <client>s *)
let client_list = Build.opt_list client comma

(* View: shell_command *)
let shell_command = [ label "shell_command"
                    . sto_to_eol ]

(* View: entry *)
let entry = [ seq "line"
            . daemon_list
            . (except daemon_list)?
            . colon
            . client_list
            . (except client_list)?
            . (colon . shell_command)?
            . Util.eol ]

(************************************************************************
 * Group:                 LENS AND FILTER
 *************************************************************************)

(* View: lns *)
let lns = (Util.empty | Util.comment | entry)*

(* View: filter *)
let filter = incl "/etc/hosts.allow"
           . incl "/etc/hosts.deny"

let xfm = transform lns filter
