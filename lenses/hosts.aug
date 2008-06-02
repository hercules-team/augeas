(* Parsing /etc/hosts *)

module Hosts =
  autoload xfm

  let sep_tab = Util.del_ws_tab
  let sep_spc = Util.del_ws_spc

  let eol = Util.del_str "\n"

  let comment = [ del /(#.*|[ \t]*)\n/ "\n" ]
  let word = /[^# \n\t]+/
  let record = [ seq "host" . [ label "ipaddr" . store  word ] . sep_tab .
                              [ label "canonical" . store word ] .
                              [ label "alias" . sep_spc . store word ]*
                 . eol ]

  let lns = ( comment | record ) *

  let xfm = transform lns (incl "/etc/hosts")

(* Local Variables: *)
(* mode: caml *)
(* End: *)


