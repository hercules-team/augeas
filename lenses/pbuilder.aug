(*
Module: Pbuilder
 Parses /etc/pbuilderrc, /etc/pbuilder/pbuilderrc

Author: Raphael Pinson <raphink@gmail.com>

About: Reference
  Pbuilderrc is a standard shellvars file.

About: License
  This file is licensed under the GPL.

About: Configuration files
  This lens applies to /etc/pbuilderrc and /etc/pbuilder/pbuilderrc.
  See <filter>.
*)

module Pbuilder =

autoload xfm

let filter = incl "/etc/pbuilder/pbuilderrc"
           . incl "/etc/pbuilderrc"

let lns    = Shellvars.lns

let xfm    = transform lns filter
