(*
Module: Sysctl
  Parses /etc/sysctl.conf and /etc/sysctl.d/*

Author: David Lutterkort <lutter@redhat.com>

About: Reference

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/sysctl.conf and /etc/sysctl.d/*. See <filter>.

About: Examples
   The <Test_Sysctl> file contains various examples and tests.
*)

module Sysctl =
autoload xfm

(* Variable: filter *)
let filter = incl "/etc/sysctl.conf"
           . incl "/etc/sysctl.d/*"
           . excl "/etc/sysctl.d/README"
           . excl "/etc/sysctl.d/README.sysctl"
           . Util.stdexcl

(* View: comment *)
let comment = Util.comment_generic /[ \t]*[#;][ \t]*/ "# "

(* View: lns
     The sysctl lens *)
let lns = (Util.empty | comment | Simplevars.entry)*

let xfm = transform lns filter
