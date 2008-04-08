(* Parsing /etc/hosts *)

module Hosts =

  let sep = del /[ \t]+/
  let sep_tab = sep "\t"
  let sep_spc = sep " "

  let eol = Util.del_str "\n"

  let comment = [ del /#.*\n/ "# " ]
  let word = /[^# \n\t]+/
  let record = [ seq "line" . [ label "ipaddr" . store  word ] . sep_tab .
                              [ label "canonical" . store word ] .
                              [ label "aliases" .
                                Util.split (store word) sep_spc
                              ] .
                 eol ]

  let lns = ( comment | record ) *

  (* let files = transform top (incl "/etc/hosts") "/system/config/hosts" *)

(* Local Variables: *)
(* mode: caml *)
(* End: *)


