(*
Module: Rsyslog
  Parses /etc/rsyslog.conf

Author: Raphael Pinson <raphael.pinsons@camptocamp.com>

About: Reference
  This lens tries to keep as close as possible to `man 5 rsyslog.conf` where possible.

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/rsyslog.conf. See <filter>.

About: Examples
   The <Test_Rsyslog> file contains various examples and tests.
*)
module Rsyslog =

autoload xfm

let macro_rx = /[^,# \n\t][^#\n]*[^,# \n\t]|[^,# \n\t]/
let macro = [ key /$[A-Za-z0-9]+/ . Sep.space . store macro_rx . Util.comment_or_eol ]

(* View: users
   Map :omusrmsg: and a list of users, or a single *
*)
let omusrmsg = Util.del_str ":omusrmsg:" .
                 Syslog.label_opt_list_or "omusrmsg" (store Syslog.word)
                                          Syslog.comma "*"

(* View: file_tmpl
   File action with a specified template *)
let file_tmpl = Syslog.file . [ label "template" . Util.del_str ";" . store Rx.word ]

let action = Syslog.action | omusrmsg | file_tmpl

(* View: entry
   An entry contains selectors and an action
*)
let entry = [ label "entry" . Syslog.selectors . Syslog.sep_tab .
              [ label "action" . action ] . Util.eol ]

(* View: prop_filter
   Parses property-based filters, which start with ":" and the property name *)
let prop_filter =
     let sep = Sep.comma . Util.del_ws_spc
  in let prop_name = [ Util.del_str ":" . label "property" . store Rx.word ]
  in let prop_oper = [ label "operation" . store /[A-Za-z!-]+/ ]
  in let prop_val  = [ label "value" . Quote.do_dquote (store /[^\n"]*/) ]
  in let prop_act  = [ label "action" . action ]
  in [ label "filter" . prop_name . sep . prop_oper . sep . prop_val .
       Sep.space . prop_act . Util.eol ]

let entries = ( Syslog.empty | Syslog.comment | entry | macro | prop_filter )*

let lns = entries . ( Syslog.program | Syslog.hostname )*

let filter = incl "/etc/rsyslog.conf"

let xfm = transform lns filter

