(*
Module: Known_Hosts
  Parses SSH known_hosts files

Author: Raphaël Pinson <raphink@gmail.com>

About: Reference
  This lens ensures that conf files included in ActiveMQ /FuseMQ are properly
  handled by Augeas.

About: License
  This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
  Sample usage of this lens in augtool:

    * Get a key by name from ssh_known_hosts
      > print /files/etc/ssh_known_hosts/*[.="foo.example.com"]
      ...

    * Change a host's key
      > set /files/etc/ssh_known_hosts/*[.="foo.example.com"]/key "newkey"

About: Configuration files
  This lens applies to SSH known_hosts files. See <filter>.

*)

module Known_Hosts =

autoload xfm

(* View: entry
     A known_hosts entry *)
let entry =
  let alias = [ label "alias" . store Rx.neg1 ]
  in [ Util.indent . seq "entry" . store Rx.neg1
     . (Sep.comma . Build.opt_list alias Sep.comma)?
     . Sep.space
     . [ label "type" . store Rx.no_spaces ]
     . Sep.space
     . [ label "key" . store Rx.no_spaces ]
     . Util.eol ]

(* View: lns
     The known_hosts lens *)
let lns = (Util.empty | Util.comment | entry)*

(* Variable: filter *)
let filter = incl "/etc/ssh/ssh_known_hosts"
           . incl (Sys.getenv("HOME") . "/.ssh/known_hosts")

let xfm = transform lns filter
