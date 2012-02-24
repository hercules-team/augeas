(*
Module: Up2date
  Parses /etc/sysconfig/rhn/up2date

Author: Raphael Pinson <raphink@gmail.com>

About: Reference
  This lens tries to keep as close as possible to `man 5 up2date` where possible.

About: License
   This file is licenced under the LGPLv2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/sysconfig/rhn/up2date. See <filter>.

About: Examples
   The <Test_Up2date> file contains various examples and tests.
*)


module Up2date =

autoload xfm

let key_re = /[^=# \t\n]+/

(* View: entry *)
let entry =
      let multi_value = [ seq "multi" . store /[^ \t\n;][^\n;]*[^ \t\n;]|[^ \t\n;]/ ]
   in let multi_entry  = counter "multi"
        . Build.opt_list multi_value Sep.semicolon . del /;?/ ""
   in [ seq "entry" . store key_re . Sep.equal
        . multi_entry? . Util.eol ]

(* View: lns *)
let lns = (Util.empty | Util.comment | entry)*

(* Variable: filter *)
let filter = incl "/etc/sysconfig/rhn/up2date"

let xfm = transform lns filter
