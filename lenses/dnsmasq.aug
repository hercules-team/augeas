(* Dnsmasq module for Augeas
 Author: Free Ekanayaka <free@64studio.com>

 Reference: man dnsmasq (8)

 "Format is one option per line, legal options are the same
  as the long options legal on the command line. See
 "/usr/sbin/dnsmasq --help" or "man 8 dnsmasq" for details."

*)

module Dnsmasq =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol        = Util.eol
let spc        = Util.del_ws_spc
let comment    = Util.comment
let empty      = Util.empty

let sep_eq     = del /=/ "="
let sto_to_eol = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let entry_re   = /[A-Za-z0-9._-]+/
let entry      = [ key entry_re . (sep_eq . sto_to_eol)? . eol ]

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns = (comment|empty|entry) *

let filter            = incl "/etc/dnsmasq.conf"
                      . incl "/etc/dnsmasq.d/*"
                      . excl ".*"
                      . Util.stdexcl

let xfm                = transform lns filter
