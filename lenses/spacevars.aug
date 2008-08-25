(* Spacevars module for Augeas
 Author: Free Ekanayaka <free@64studio.com>

 Reference: man interfaces
 This is a generic lens for simple key/value configuration files where
 keys and values are separated by a sequence of spaces or tabs.

*)

module Spacevars =

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol     = Util.eol
let spc     = Util.del_ws_spc
let comment = Util.comment
let empty   = Util.empty

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let keyword = key /[A-Za-z0-9\._-]+(\[[0-9]+\])?/
let value   = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/
let entry   = [ keyword . spc . value . eol ]

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns     = (comment|empty|entry) *
