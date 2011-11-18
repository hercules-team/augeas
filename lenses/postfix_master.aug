(* Postfix_Master module for Augeas
 Author: Free Ekanayaka <free@64studio.com>

 Reference:

*)

module Postfix_Master =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol        = Util.eol
let ws         = del /[ \t\n]+/ " "
let comment    = Util.comment
let empty      = Util.empty

let word       = /[A-Za-z0-9_.:-]+/
let words      = /[A-Za-z0-9$(){}=!_.,:@-][A-Za-z0-9$!(){} ="\/_.,:@-]*[A-Za-z0-9$!(){}="\/_.,:@-]/

let bool       = /y|n|-/
let integer    = /([0-9]+|-)\??/
let command   = words . (/[ \t]*\n[ \t]+/ . words)*

let field (l:string) (r:regexp)
               = [ label l . store r ]

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let entry     = [ key word . ws
                . field "type"         /inet|unix|fifo|pass/  . ws
                . field "private"      bool                   . ws
                . field "unprivileged" bool                   . ws
                . field "chroot"       bool                   . ws
                . field "wakeup"       integer                . ws
                . field "limit"        integer                . ws
                . field "command"      command
                . eol ]

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = (comment|empty|entry) *

let filter     = incl "/etc/postfix/master.cf"
               . Util.stdexcl

let xfm        = transform lns filter
