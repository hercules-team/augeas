(* Monit module for Augeas
 Author: Free Ekanayaka <free@64studio.com>

 Reference: man monit (1), section "HOW TO MONITOR"

 "A monit control file consists of a series of service entries and
  global option statements in a free-format, token-oriented syntax.

  Comments begin with a # and extend through the end of the line. There
  are three kinds of tokens in the control file: grammar keywords, numbers
  and strings. On a semantic level, the control file consists of three
  types of statements:

  1. Global set-statements
      A global set-statement starts with the keyword set and the item to
      configure.

  2. Global include-statement
      The include statement consists of the keyword include and a glob
      string.

  3. One or more service entry statements.
       A service entry starts with the keyword check followed by the
       service type"

*)

module Monit =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol        = Util.eol
let spc        = Util.del_ws_spc
let comment    = Util.comment
let empty      = Util.empty

let sto_to_eol = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/
let sto_to_spc = store /[^ \t\n]+/

let word       = /[A-Za-z0-9_.-]+/
let value      = [ key word . spc . sto_to_eol . eol ]

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

(* set statement *)
let set        = [ key "set" . spc . value  ]

(* include statement *)
let include    = [ key "include" . spc . sto_to_eol . eol ]

(* service statement *)
let service    = [ key "check" . spc . value . (spc . value)+ ]

let entry      = (set|include|service)

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = (comment|empty|entry) *

let filter     = incl "/etc/monit/monitrc"
               . Util.stdexcl

let xfm        = transform lns filter
