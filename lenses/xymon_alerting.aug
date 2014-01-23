(*
Module: Xymon_Alerting
  Parses xymon alerting files 

Author: Francois Maillard <fmaillard@gmail.com>

About: Reference
  This lens tries to keep as close as possible to `man 5 alerts.cfg` where possible.

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Not supported
   File inclusion are not followed

About: Configuration files
   This lens applies to /etc/xymon/alerts.d/*.cfg and /etc/xymon/alerts.cfg. See <filter>.

About: Examples
   The <Test_Xymon_Alerting> file contains various examples and tests.
*)

module Xymon_Alerting =
    autoload xfm

    (************************************************************************
     * Group:                 USEFUL PRIMITIVES
     *************************************************************************)

    (* View: store_word *)
    let store_word  = store /[^ =\t\n#]+/

    (* View: comparison The greater and lesser than operators *)
    let comparison  = store /[<>]/

    (* View: equal *)
    let equal       = del /=/ "="

    (* View: ws *)
    let ws          = del /[ \t]+?/ " "

    (* View: eol *)
    let eol         = del /\n/ "\n"

    (* View: ws_or_eol *)
    let ws_or_eol   = del /[ \t\n]+?/ " "

    (* View: comment *)
    let comment = Util.comment

    (* View: empty *)
    let empty   = Util.empty

    (* View: include *)
    let include         = [ key "include" . ws . store_word . eol ]

    (************************************************************************
     * Group:                 MACRO DEFINITION
     *************************************************************************)

    (* View: macrodefinition
         A string that starts with $ and that is assigned something *)
    let macrodefinition = [ key /\$[^ =\t\n#\/]+/ . equal . store /.*/ . eol ]


    (************************************************************************
     * Group:                 FILTERS 
     *************************************************************************)

    (* View: page
         The page filter definition *)
    let page      = [ ws . key "PAGE" . equal . store_word . ws ]
    (* View: expage
         The expage filter definition *)
    let expage    = [ ws . key "EXPAGE" . equal . store_word . ws ]
    (* View: group
         The group filter definition *)
    let group     = [ ws . key "GROUP" . equal . store_word . ws ]
    (* View: exgroup
         The exgroup filter definition *)
    let exgroup   = [ ws . key "EXGROUP" . equal . store_word . ws ]
    (* View: host
         The host filter definition *)
    let host      = [ ws . key "HOST" . equal . store_word . ws ]
    (* View: exhost
         The exhost filter definition *)
    let exhost    = [ ws . key "EXHOST" . equal . store_word . ws ]
    (* View: service
         The service filter definition *)
    let service   = [ ws . key "SERVICE" . equal . store_word . ws ]
    (* View: exservice
         The exservice filter definition *)
    let exservice = [ ws . key "EXSERVICE" . equal . store_word . ws ]
    (* View: color
         The color filter definition *)
    let color     = [ ws . key "COLOR" . equal . store_word . ws ]
    (* View: time
         The time filter definition *)
    let time      = [ ws . key "TIME" . equal . store_word . ws ]
    (* View: duration
         The duration filter definition *)
    let duration  = [ ws . key "DURATION" . [ label "operator" . comparison ] . [ label "value" . store_word ] . ws ]
    (* View: recover
         The recover filter definition *)
    let recover   = [ ws . key "RECOVER" . ws ]
    (* View: notice
         The notice filter definition *)
    let notice    = [ ws . key "NOTICE" . ws ]

    (* View: filters
         Filters are made out of any of the above filter definitions *)
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


    (************************************************************************
     * Group:                 RECIPIENTS
     *************************************************************************)

    (* View: mail
         The mail recipient definition *)
    let mail      = [ key "MAIL" . ws . store_word . ws . [ label "filters" . filters* ] . ws_or_eol ]
    (* View: script
         The script recipient definition *)
    let script    = [ key "SCRIPT" . ws . [ label "script" . store_word ] . ws . [ label "recipient" . store_word ] . ws . [ label "filters" . filters* ] . ws_or_eol ]
    (* View: ignore
         The ignore recipient definition *)
    let ignore    = [ key "IGNORE" . ws . [ label "filters" . filters* ] . ws_or_eol ]
    (* View: format
         The format recipient definition *)
    let format    = [ key "FORMAT" . equal . store_word . ws . [ label "filters" . filters* ] . ws_or_eol ]
    (* View: repeat
         The repeat recipient definition *)
    let repeat    = [ key "REPEAT" . equal . store_word . ws . [ label "filters" . filters* ] . ws_or_eol ]
    (* View: unmatched
         The unmatched recipient definition *)
    let unmatched = [ key "UNMATCHED" . ws . [ label "filters" . filters* ] . ws_or_eol ]
    (* View: stop
         The stop recipient definition *)
    let stop      = [ key "STOP" . ws . [ label "filters" . filters* ] . ws_or_eol ]
    (* View: macro
         The macro recipient definition *)
    let macro     = [ key /\$[^ =\t\n#\/]+/ . ws . [ label "filters" . filters* ] . ws_or_eol ]

    (* View: recipients
         Recipients are made out of any of the above recipient definitions *)
    let recipients =
        mail |
        script |
        ignore |
        format |
        repeat |
        unmatched |
        stop |
        macro


    (************************************************************************
     * Group:                 RULES
     *************************************************************************)

    (* View: rule
         Rules are made of filters and then recipients sperarated by a whitespace *)
    let rule = [ seq "rules" . [ label "filters" . filters+ ] . ws_or_eol . [ label "recipients" . ( recipients )+ ] . eol ] 

    (* View: lns
         The Xymon_Alerting lens *)
    let lns = ( rule | macrodefinition | include | empty | comment )*

    (* Variable: filter *)
    let filter = incl "/etc/xymon/alerts.d/*.cfg"
               . incl "/etc/xymon/alerts.cfg"
               . Util.stdexcl

    let xfm = transform lns filter

