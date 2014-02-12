(*
Module: Chrony
  Parses the chrony config file

Author: Pat Riehecky <riehecky@fnal.gov>

About: Reference
  This lens tries to keep as close as possible to chrony config syntax
  http://chrony.tuxfamily.org/manual.html#Configuration-file


About: Limitations
  Does not (currently) support:
    - include
    - manual
    - refclock
    - tempcomp

About: License
  This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
  To be documented

About: Configuration files
  This lens applies to:
    /etc/chrony.conf

  See <filter>.
*)

module Chrony =
  autoload xfm

(************************************************************************
 * Import provided expressions
 ************************************************************************)
    let empty   = Util.empty
    let eol     = Util.eol

    let space   = Sep.space

    let email_addr = Rx.email_addr
    let word       = Rx.word
    let integer    = Rx.integer
    let decimal    = Rx.decimal
    let ip         = Rx.ip
    let address_re = Rx.ip | Rx.hostname

    let number = integer | decimal


(************************************************************************
 * Create required expressions
 ************************************************************************)
    (* Chrony comments start with: ! ; # or %   from 4.2.1 of their doc *)
    (* Comments must be on their own line *)
    let comment = Util.comment_generic /[ \t]*[!;#%][ \t]*/ "# "

    (* No spaces or comment characters *)
    let no_space   = /[^ \t\r\n!;#%]+/

    (* Server/Peer options with values *)
    let cmd_options = /key|maxdelay|maxdelaydevratio|maxdelayratio|maxpoll|minpoll|polltarget|port|presend/

    (* Server/Peer options without values *)
    let cmd_flags = /auto_offline|iburst|noselect|offline|prefer/

    (* Server/Peer options values *)
    let server_peer = /server|peer/

    (* Options without values *)
    let flags = /dumponexit|generatecommandkey|lock_all|noclientlog|rtconutc|rtcsync/

    (* log has a specific options list *)
    let log_flags = /measurments|statistics|tracking|rtc|refclocks|tempcomp/

    (* Options with single values *)
    let simple_keys = /acquisitionport|allow|bindaddress|bindcmdaddress|cmdallow|cmddeny|combinelimit|commandkey|cmdport|corrtimeratio|deny|driftfile|dumpdir|keyfile|leapsectz|linux_hz|linux_freq_scale|logbanner|logchange|logdir|maxclockerror|maxsamples|maxupdateskew|minsamples|clientloglimit|pidfile|port|reselectdist|rtcdevice|rtcfile|sched_priority|stratumweight|user/

    (* Options with multiple values *)
    (*
      Each of these gets their own stanza
      - broadcast <interval> <address> <optional port>
      - fallbackdrift <min> <max>
      - initstepslew <threshold> <addr> <optional extra addrs>
      - local stratum <int>
      - mailonchange <emailaddress> <threshold>
      - makestep <threshold> <limit>
      - maxchange <threshold> <delay> <limit>
    *)

(************************************************************************
 * Make some sub-lenses for including later on
 ************************************************************************)
    let host_flags = [ space . key cmd_flags ]
    let host_options = [ space . key cmd_options . space . store integer ]
    let log_flag_list = [ space . key log_flags ]
    let store_address = [ label "address" . store address_re ]

(************************************************************************
 * Parse out sections
 ************************************************************************)
    (* match all flags *)
    let all_flags = Build.flag_line flags

    (* options with only one arg *)
    let kv = Build.key_value_line_comment simple_keys space (store no_space) comment

    (* ntp servers/peers *)
    let host_list = [ Util.indent . key server_peer
                         . space . store address_re
                         . ( host_flags | host_options )*
                         . eol ]

    (* log has a specific options list *)
    let log_list = [ Util.indent . key "log" . log_flag_list+ . eol ]

    (* broadcast has specific syntax *)
    let bcast = [ Util.indent . key "broadcast"
                      . space . [ label "interval" . store integer ]
                      . space . store_address
                      . ( space . [ label "port" . store integer] | eol) ]

    (* fallbackdrift has specific syntax *)
    let fdrift = [ Util.indent . key "fallbackdrift"
                      . space . [ label "min" . store integer ]
                      . space . [ label "max" . store integer ]
                      . eol ]

    (* initstepslew has specific syntax *)
    let istepslew = [ Util.indent . key "initstepslew" 
                         . space . [ label "threshold" . store number ]
                         . ( space . store_address )+
                         . eol ]

    (* local has specific syntax *)
    let local = [ Util.indent . key "local" . space
                     . [ key "stratum" . space . store integer ]
                     . eol ]

    (* mailonchange has specific syntax *)
    let email = [ Util.indent . key "mailonchange" . space
                     . [ label "emailaddress" . store email_addr ]
                     . space
                     . [ label "threshold" . store number ]
                     . eol ]

    (* makestep has specific syntax *)
    let makestep = [ Util.indent . key "makestep"
                      . space
                      . [ label "threshold" . store number ]
                      . space
                      . [ label "limit" . store integer ]
                      . eol ]

    (* maxchange has specific syntax *)
    let maxchange = [ Util.indent . key "maxchange"
                      . space
                      . [ label "threshold" . store number ]
                      . space
                      . [ label "delay" . store integer ]
                      . space
                      . [ label "limit" . store integer ]
                      . eol ]

let settings = ( host_list | log_list | bcast | fdrift | istepslew | local | email | makestep | maxchange | kv | all_flags )

let lns = ( empty | comment | settings )*

let filter = (incl "/etc/chrony.conf")

let xfm = transform lns filter

