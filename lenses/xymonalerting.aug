(* xymon alerting module for Augeas                *)
(* Author: Francois Maillard <fmaillard@gmail.com> *)
(*                                                 *)
(* Not supported :                                 *)
(*   file inclusion are not followed               *)
(*                                                 *)
(* Supported :                                     *)
(*                                                 *)
(* Todo :                                          *)
(*                                                 *)

module XymonAlerting =
    autoload xfm

    let store_word  = store /[^ =\t\n#]+/
    let comparison  = store /[<>]/
    let equal       = del /=/ "="
    let ws          = del /[ \t]+?/ " "
    let eol         = del /\n/ "\n"
    let ws_or_eol   = del /[ \t\n]+?/ " "

    (* Define comments and empty lines *)
    let comment = Util.comment
    let empty   = Util.empty

    let include         = [ key "include" . ws . store_word . eol ]
    let macrodefinition = [ key /\$[^ =\t\n#\/]+/ . equal . store /.*/ . eol ]

    (* Define the available filters in the rules *)
    let page      = [ ws . key "PAGE" . equal . store_word . ws ]
    let expage    = [ ws . key "EXPAGE" . equal . store_word . ws ]
    let group     = [ ws . key "GROUP" . equal . store_word . ws ]
    let exgroup   = [ ws . key "EXGROUP" . equal . store_word . ws ]
    let host      = [ ws . key "HOST" . equal . store_word . ws ]
    let exhost    = [ ws . key "EXHOST" . equal . store_word . ws ]
    let service   = [ ws . key "SERVICE" . equal . store_word . ws ]
    let exservice = [ ws . key "EXSERVICE" . equal . store_word . ws ]
    let color     = [ ws . key "COLOR" . equal . store_word . ws ]
    let time      = [ ws . key "TIME" . equal . store_word . ws ]
    let duration  = [ ws . key "DURATION" . [ label "operator" . comparison ] . [ label "value" . store_word ] . ws ]
    let recover   = [ ws . key "RECOVER" . ws ]
    let notice    = [ ws . key "NOTICE" . ws ]

    let filters = 
        page |
        expage |
        group |
        exgroup |
        host |
        exhost |
        service |
        exservice |
        color |
        time |
        duration |
        recover |
        notice

    (* Recipients *)
    let mail      = [ key "MAIL" . ws . store_word . ws . [ label "filters" . filters* ] . ws_or_eol ]
    let script    = [ key "SCRIPT" . ws . [ label "script" . store_word ] . ws . [ label "recipient" . store_word ] . ws . [ label "filters" . filters* ] . ws_or_eol ]
    let ignore    = [ key "IGNORE" . ws . [ label "filters" . filters* ] . ws_or_eol ]
    let format    = [ key "FORMAT" . equal . store_word . ws . [ label "filters" . filters* ] . ws_or_eol ]
    let repeat    = [ key "REPEAT" . equal . store_word . ws . [ label "filters" . filters* ] . ws_or_eol ]
    let unmatched = [ key "UNMATCHED" . ws . [ label "filters" . filters* ] . ws_or_eol ]
    let stop      = [ key "STOP" . ws . [ label "filters" . filters* ] . ws_or_eol ]
    let macro     = [ key /\$[^ =\t\n#\/]+/ . ws . [ label "filters" . filters* ] . ws_or_eol ]

    let recipients =
        mail |
        script |
        ignore |
        format |
        repeat |
        unmatched |
        stop |
        macro

    (* A rule is made of entries sperarated by a whitespace *)
    let rule = [ seq "rules" . [ label "filters" . filters+ ] . ws_or_eol . [ label "recipients" . ( recipients )+ ] . eol ] 

    (* Define lens *)
    let lns = ( rule | macrodefinition | include | empty | comment )*

    (* let filter = incl "pfs_.*.cfg" *)
    let filter = incl "/etc/xymon/alerts.d/*.cfg"
               . incl "/etc/xymon/alerts.cfg"
               . Util.stdexcl

    let xfm = transform lns filter

