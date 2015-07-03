(*
Module: Rhsm
  Parses subscription-manager config files

Author: Dominic Cleal <dcleal@redhat.com>

About: Reference
  This lens tries to keep as close as possible to rhsm.conf(5)

About: License
  This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
  To be documented

About: Configuration files
  This lens applies to:
    /etc/rhsm/rhsm.conf

  See <filter>.
*)

module Rhsm =
  autoload xfm

let lns     = IniFile.lns_loose_multiline

let filter = incl "/etc/rhsm/rhsm.conf"

let xfm = transform lns filter
