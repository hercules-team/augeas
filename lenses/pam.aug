(* Proces /etc/pam.d *)
module Pam =
  autoload xfm

  let eol = Util.del_str "\n"

  (* For the control syntax of [key=value ..] we could split the key value *)
  (* pairs into an array and generate a subtree control/N/KEY = VALUE      *)
  let control = /(\[[^]#\n]*\]|[^[ \t][^ \t]*)/
  let word = /[^# \t\n]+/
  let opts = /[^#\n \t][^#\n]*/

  let comment = [ del /[ \t]*(#.*)?\n/ "#\n" ]

  let record = [ seq "record" .
                   [ label "type" . store word ] .
                   Util.del_ws_tab .
                   [ label "control" . store control] .
                   Util.del_ws_tab .
                   [ label "module" . store word ] .
                   ( [ Util.del_ws_tab . label "opts" . store opts ] )? .
                 eol
               ]
  let lns = ( comment | record ) *

  let xfm = transform lns ((incl "/etc/pam.d/*") . Util.stdexcl)

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
