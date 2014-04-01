(*
Module: Pam
  Parses /etc/pam.conf and /etc/pam.d/* service files

Author: David Lutterkort <lutter@redhat.com>

About: Reference
  This lens tries to keep as close as possible to `man pam.conf` where
  possible.

About: Licence
  This file is licensed under the LGPL v2+, like the rest of Augeas.

About: Lens Usage

About: Configuration files
  This lens autoloads /etc/pam.d/* for service specific files. See <filter>.
  It provides a lens for /etc/pam.conf, which is used in the PamConf module.
*)
module Pam =
  autoload xfm

  let eol = Util.eol
  let indent = Util.indent

  (* For the control syntax of [key=value ..] we could split the key value *)
  (* pairs into an array and generate a subtree control/N/KEY = VALUE      *)
  let control = /(\[[^]#\n]*\]|[^[ \t][^ \t]*)/
  let word = /[^# \t\n]+/
  (* Allowed types *)
  let types = /(auth|session|account|password)/i

  (* This isn't entirely right: arguments enclosed in [ .. ] can contain  *)
  (* a ']' if escaped with a '\' and can be on multiple lines ('\')       *)
  let argument = /(\[[^]#\n]+\]|[^[#\n \t][^#\n \t]*)/

  let comment = Util.comment
  let comment_or_eol = Util.comment_or_eol
  let empty   = Util.empty


  (* Not mentioned in the man page, but Debian uses the syntax             *)
  (*   @include module                                                     *)
  (* quite a bit                                                           *)
  let include = [ indent . Util.del_str "@" . key "include" .
                  Util.del_ws_spc . store word . eol ]

  (* Shared with PamConf *)
  let record = [ label "optional" . del "-" "-" ]? .
               [ label "type" . store types ] .
               Util.del_ws_tab .
               [ label "control" . store control] .
               Util.del_ws_tab .
               [ label "module" . store word ] .
               [ Util.del_ws_tab . label "argument" . store argument ]* .
               comment_or_eol

  let record_svc = [ seq "record" . indent . record ]

  let lns = ( empty | comment | include | record_svc ) *

  let filter = incl "/etc/pam.d/*"
             . excl "/etc/pam.d/allow.pamlist"
             . excl "/etc/pam.d/README"
             . Util.stdexcl

  let xfm = transform lns filter

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
