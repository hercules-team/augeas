(*
Module: Chrony
  Parses the chrony config file

Author: Pat Riehecky <riehecky@fnal.gov>

About: Reference
  This lens tries to keep as close as possible to chrony config syntax

  See http://chrony.tuxfamily.org/manual.html#Configuration-file

About: Limitations
  Does not (currently) support
    - include
    - manual
    - refclock
    - tempcomp

About: License
  This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
  To be documented

About: Configuration files
  This lens applies to /etc/chrony.conf

  See <filter>.
*)

module Chrony =
  autoload xfm

(************************************************************************
 * Group: Import provided expressions
 ************************************************************************)
    (* View: empty *)
    let empty   = Util.empty

    (* View: eol *)
    let eol     = Util.eol

    (* View: space *)
    let space   = Sep.space

    (* Variable: email_addr *)
    let email_addr = Rx.email_addr

    (* Variable: word *)
    let word       = Rx.word

    (* Variable: integer *)
    let integer    = Rx.integer

    (* Variable: decimal *)
    let decimal    = Rx.decimal

    (* Variable: ip *)
    let ip         = Rx.ip

(************************************************************************
 * Group: Create required expressions
 ************************************************************************)
    (* Variable: number *)
    let number = integer | decimal

    (* Variable: address_re *)
    let address_re = Rx.ip | Rx.hostname

    (*
       View: comment
            from 4.2.1 of the upstream doc
            Chrony comments start with: ! ; # or % and must be on their own line
    *)
    let comment = Util.comment_generic /[ \t]*[!;#%][ \t]*/ "# "

    (* Variable: no_space
         No spaces or comment characters
    *)
    let no_space   = /[^ \t\r\n!;#%]+/

    (* Variable: cmd_options
         Server/Peer options with values
    *)
    let cmd_options = "key"
                    | /maxdelay((dev)?ratio)?/
                    | /(min|max)poll/
                    | "polltarget"
                    | "port"
                    | "presend"

    (* Variable: cmd_flags
         Server/Peer options without values
    *)
    let cmd_flags = "auto_offline"|"iburst"|"noselect"|"offline"|"prefer"

    (* Variable: server_peer
         Server/Peer key names
    *)
    let server_peer = "server"|"peer"

    (* Variable: flags
         Options without values
    *)
    let flags = "dumponexit"
              | "generatecommandkey"
              | "lock_all"
              | "noclientlog"
              | "rtconutc"
              | "rtcsync"

    (* Variable: log_flags
        log has a specific options list
    *)
    let log_flags = /measurements|statistics|tracking|rtc|refclocks|tempcomp/

    (* Variable: simple_keys
         Options with single values
    *)
    let simple_keys = "acquisitionport" | "allow" | "bindaddress"
                    | "bindcmdaddress" | "cmdallow" | "cmddeny"
                    | "combinelimit" | "commandkey" | "cmdport"
                    | "corrtimeratio" | "deny" | "driftfile"
                    | "dumpdir" | "keyfile" | "leapsectz" | "linux_hz"
                    | "linux_freq_scale" | "logbanner" | "logchange"
                    | "logdir" | "maxclockerror" | "maxsamples"
                    | "maxupdateskew" | "minsamples" | "clientloglimit"
                    | "pidfile" | "port" | "reselectdist" | "rtcdevice"
                    | "rtcfile" | "sched_priority" | "stratumweight" | "user"

(************************************************************************
 * Group: Make some sub-lenses for use in later lenses
 ************************************************************************)
    (* View: host_flags *)
    let host_flags = [ space . key cmd_flags ]
    (* View: host_options *)
    let host_options = [ space . key cmd_options . space . store integer ]
    (* View: log_flag_list *)
    let log_flag_list = [ space . key log_flags ]
    (* View: store_address *)
    let store_address = [ label "address" . store address_re ]

(************************************************************************
 * Group: Lenses for parsing out sections
 ************************************************************************)
    (* View: all_flags
        match all flags using Build.flag_line
    *)
    let all_flags = Build.flag_line flags

    (* View: kv
        options with only one arg can be directly mapped to key = value
    *)
    let kv = Build.key_value_line_comment simple_keys space (store no_space) comment

    (* Property: Options with multiple values
    
      Each of these gets their own parsing block
      - server|peer <address> <options>
      - log <options>
      - broadcast <interval> <address> <optional port>
      - fallbackdrift <min> <max>
      - initstepslew <threshold> <addr> <optional extra addrs>
      - local stratum <int>
      - mailonchange <emailaddress> <threshold>
      - makestep <threshold> <limit>
      - maxchange <threshold> <delay> <limit>
    *)

    (* View: host_list
        Find all ntp servers/peers and their flags/options
    *)
    let host_list = [ Util.indent . key server_peer
                         . space . store address_re
                         . ( host_flags | host_options )*
                         . eol ]

    (* View: log_list
        log has a specific options list
    *)
    let log_list = [ Util.indent . key "log" . log_flag_list+ . eol ]

    (* View: bcast
         broadcast has specific syntax
    *)
    let bcast = [ Util.indent . key "broadcast"
                      . space . [ label "interval" . store integer ]
                      . space . store_address
                      . ( space . [ label "port" . store integer] | eol) ]

    (* View: fdrift
         fallbackdrift has specific syntax
    *)
    let fdrift = [ Util.indent . key "fallbackdrift"
                      . space . [ label "min" . store integer ]
                      . space . [ label "max" . store integer ]
                      . eol ]

    (* View: istepslew
         initstepslew has specific syntax
    *)
    let istepslew = [ Util.indent . key "initstepslew" 
                         . space . [ label "threshold" . store number ]
                         . ( space . store_address )+
                         . eol ]

    (* View: local
         local has specific syntax
    *)
    let local = [ Util.indent . key "local" . space
                     . [ key "stratum" . space . store integer ]
                     . eol ]

    (* View: email
         mailonchange has specific syntax
    *)
    let email = [ Util.indent . key "mailonchange" . space
                     . [ label "emailaddress" . store email_addr ]
                     . space
                     . [ label "threshold" . store number ]
                     . eol ]

    (* View: makestep
         makestep has specific syntax
    *)
    let makestep = [ Util.indent . key "makestep"
                      . space
                      . [ label "threshold" . store number ]
                      . space
                      . [ label "limit" . store integer ]
                      . eol ]

    (* View: maxchange
         maxchange has specific syntax
    *)
    let maxchange = [ Util.indent . key "maxchange"
                      . space
                      . [ label "threshold" . store number ]
                      . space
                      . [ label "delay" . store integer ]
                      . space
                      . [ label "limit" . store integer ]
                      . eol ]

(************************************************************************
 * Group: Final lense summary
 ************************************************************************)
(* View: settings
 *   All supported chrony settings
 *)
let settings = host_list | log_list | bcast | fdrift | istepslew
             | local | email | makestep | maxchange | kv | all_flags

(*
 * View: lns
 *   The crony lens
 *)
let lns = ( empty | comment | settings )*

(* View: filter
 *   The files parsed by default
 *)
let filter = incl "/etc/chrony.conf"

let xfm = transform lns filter

