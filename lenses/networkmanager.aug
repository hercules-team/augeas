(*
Module: NetworkManager
  Parses /etc/NetworkManager/system-connections/*

Author: Raphael Pinson <raphael.pinson@camptocamp.com>

About: License
   This file is licenced under the LGPLv2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/NetworkManager/system-connections/*. See <filter>.

About: Examples
   The <Test_NetworkManager> file contains various examples and tests.
*)

module NetworkManager =
autoload xfm

(* View:lns
     Same format as Puppet's IniFile *)
let lns = Puppet.lns

(* Variable: filter *)
let filter = incl "/etc/NetworkManager/system-connections/*"

let xfm = transform lns filter
