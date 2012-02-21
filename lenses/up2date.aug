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

(* View: entry_comment *)
let entry_comment = [ label "#comment"
                    . Util.del_str "[comment]="
                    . store Rx.space_in ]

(* View: entry_generic *)
let entry_generic (kw:regexp) (lns:lens) =
   [ key kw . ( entry_comment | ( Sep.equal . lns? ) ) . Util.eol ]

(* View: entry *)
let entry =
      let num_re = "enableProxy" | "debug" | "enableProxyAuth"
                 | "networkRetries" | "noReboot" | "retrieveOnly"
                 | "useNoSSLForPackages" | "skipNetwork"
                 | "writeChangesToLog" | "stagingContent"
                 | "stagingContentWindow"
   in let single_re = "serverURL" | "systemIdPath" | "versionOverride"
                    | "httpProxy" | "proxyUser" | "proxyPassword"
                    | "sslCACert" | "tmpDir"
   in let multi_re = "disallowConfChanges" | "hostedWhitelist"
   in let multi_entry = [ seq "multi" . store /[^ \t\n;][^\n;]*[^ \t\n;]|[^ \t\n;]/ ]
   in entry_generic num_re ( store Rx.integer )
    | entry_generic single_re ( store Rx.space_in )
    | entry_generic multi_re ( counter "multi"
         . Build.opt_list multi_entry Sep.semicolon . del /;?/ "" )

(* View: lns *)
let lns = (Util.empty | Util.comment | entry)*

(* Variable: filter *)
let filter = incl "/etc/sysconfig/rhn/up2date"

let xfm = transform lns filter
