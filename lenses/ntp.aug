(* NTP module for Augeas                      *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)
(* Status: basic settings supported           *)

module Ntp =
  autoload xfm


    (* Define useful shortcuts *)

    let eol = Util.del_str "\n"
    let sep_spc = Util.del_ws_spc
    let word = /[^,# \n\t]+/
    let num  = /[0-9]+/


    (* define comments and empty lines *)
    let comment = [ label "comment" . del /#[ \t]*/ "#" .  store /([^ \t\n][^\n]*)?/ . eol ]
    let empty   = [ del /[ \t]*\n/ "\n" ]


    (* Define generic record *)
    let record (kw:string) (value:lens) = [ key kw . sep_spc . store word . value . eol ]

    (* Define a server record *)
    let version  = [ sep_spc . key "version" . sep_spc . store word ]
    let server_record   = record "server" version

    (* Define simple settings *)
    let simple_setting (kw:string) = [ key kw . sep_spc . store word . eol ]

    let simple_settings = simple_setting "driftfile"
                        | simple_setting "logfile"
			| simple_setting "statsdir"
    
    (* Define restrict *)
    let action    = [ label "action" . sep_spc . store word ]
    let restrict_record   = record "restrict" action?

    (* Define statistics *)
    let statistics_flag (kw:string) = [ sep_spc . key kw ]

    let statistics_opts = statistics_flag "loopstats"
                        | statistics_flag "peerstats"
			| statistics_flag "clockstats"
			| statistics_flag "rawstats"

    let statistics_record = [ key "statistics" . statistics_opts* . eol ]


    (* Define filegen *)
    let filegen = del /filegen[ \t]+/ "filegen " . store word
    let filegen_opt (kw:string) = [ sep_spc . key kw . sep_spc . store word ]
    (* let filegen_flag (kw:string) = [ label kw . sep_spc . store word ] *)
    let filegen_select (kw:string) (select:regexp) = [ label kw . sep_spc . store select ]

    let filegen_opts = filegen_opt "file"
                     | filegen_opt "type"
		     | filegen_select "enable" /(en|dis)able/
		     | filegen_select "link" /(no)?link/

    let filegen_record = [ label "filegen" . filegen . filegen_opts* . eol ]


    (* Define lens *)

    let lns = ( comment | empty | server_record | restrict_record | simple_settings | statistics_record | filegen_record )*

    let filter = (incl "/etc/ntp.conf")
        . Util.stdexcl

    let xfm = transform lns filter
