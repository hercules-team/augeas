(* Parsing /etc/apt/sources.list *)

module Aptsources =
  autoload xfm

  let sep_ws = del /[ \t]+/ " "

  let eol = Util.del_str "\n"

  let comment = [ del /([ \t]*\n)|(#.*\n)/ "#\n" ]

  let word = /[^# \n\t]+/

  let record = [ Util.indent . seq "source" . [ label "type" . store word ] . sep_ws .
                                [ label "uri"  . store word ] . sep_ws .
                                [ label "distribution" . store word ]  .
                                [ label "component" . sep_ws . store word ]* .
                                del /[ \t]*(#.*)?/ ""
                 . eol ]

  let lns = ( comment | record ) *

  let filter = (incl "/etc/apt/sources.list")
      . (incl "/etc/apt/sources.list.d/*")
      . Util.stdexcl

  let xfm = transform lns filter

(* Local Variables: *)
(* mode: caml *)
(* End: *)
