(* Puppet module for Augeas
 Author: Raphael Pinson <raphink@gmail.com>

 puppet.conf is a standard INI File.
*)


module Puppet =
  autoload xfm

let comment = IniFile.comment_generic "#" "#"
let entry   = IniFile.entry_nocolon_setcomment /[a-z][a-z0-9\._-]+/ comment
let record  = IniFile.record_setcomment "section" entry comment
let lns     = IniFile.lns_setcomment record comment

let filter = (incl "/etc/puppet/puppet.conf")

let xfm = transform lns filter
