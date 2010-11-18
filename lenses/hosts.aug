(* Parsing /etc/hosts *)

module Hosts =
  autoload xfm

  let sep_tab = Util.del_ws_tab
  let sep_spc = Util.del_ws_spc

  let eol = Util.eol
  let indent = Util.indent

  let comment = Util.comment
  let comment_or_eol = Util.comment_or_eol
  let empty   = [ del /[ \t]*#?[ \t]*\n/ "\n" ]

  let word = /[^# \n\t]+/
  let record = [ seq "host" . indent .
                              [ label "ipaddr" . store  word ] . sep_tab .
                              [ label "canonical" . store word ] .
                              [ label "alias" . sep_spc . store word ]*
                 . comment_or_eol ]

  let lns = ( empty | comment | record ) *

  let xfm = transform lns (incl "/etc/hosts")
