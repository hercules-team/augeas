(* Parsing /etc/hosts *)

module Hosts =

  let sep = del /[ \t]+/
  let sep_tab = sep "\t"
  let sep_spc = sep " "

  let del_str (s:string) = del s s
  let eol = del_str "\n"
  (* What now ? *)
  let comment = [ del /#.*\n/ "# " ]
  let word = /[^# \n\t]+/
  let record = [ seq "line" . [ label "ipaddr" . store  word ] . sep_tab .
                              [ label "canonical" . store word ] .
                              [ label "aliases" . 
                                ( [ seq "aliases" . sep_spc . store word] ) * 
                              ] .
                 eol ]

  let top = ( comment | record ) *

  (* let files = transform top (incl "/etc/hosts") "/system/config/hosts" *)

(* Local Variables: *)
(* mode: caml *)
(* End: *)


