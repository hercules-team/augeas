(* Proces /etc/pam.d *)
module Pam =
  autoload xfm

  let eol = Util.eol
  let indent = Util.indent

  (* For the control syntax of [key=value ..] we could split the key value *)
  (* pairs into an array and generate a subtree control/N/KEY = VALUE      *)
  let control = /(\[[^]#\n]*\]|[^[ \t][^ \t]*)/
  let word = /[^# \t\n]+/
  (* Allowed types. FIXME: Should be case insensitive *)
  let types = /(auth|session|account|password)/

  (* This isn't entirely right: arguments enclosed in [ .. ] are allowed   *)
  (* and should be parsed as one                                           *)
  let argument = /[^#\n \t]+/

  let comment = Util.comment
  let comment_or_eol = Util.comment_or_eol
  let empty   = Util.empty


  (* Not mentioned in the man page, but Debian uses the syntax             *)
  (*   @include module                                                     *)
  (* quite a bit                                                           *)
  let include = [ indent . Util.del_str "@" . key "include" .
                  Util.del_ws_spc . store word . eol ]

  let record = [ seq "record" . indent .
                   [ label "optional" . del "-" "-" ]? .
                   [ label "type" . store types ] .
                   Util.del_ws_tab .
                   [ label "control" . store control] .
                   Util.del_ws_tab .
                   [ label "module" . store word ] .
                   [ Util.del_ws_tab . label "argument" . store argument ]* .
		 comment_or_eol
               ]
  let lns = ( empty | comment | include | record ) *

  let filter = incl "/etc/pam.conf"
             . incl "/etc/pam.d/*"
             . Util.stdexcl

  let xfm = transform lns filter

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
