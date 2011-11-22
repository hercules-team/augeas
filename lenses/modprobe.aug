(*
Module: Modprobe
  Parses /etc/modprobe.conf and /etc/modprobe.d/*

Original Author: David Lutterkort <lutter@redhat.com>

Author: Raphael Pinson <raphink@gmail.com>

About: Reference
  This lens tries to keep as close as possible to `man 5 modprobe.conf` where possible.

About: License
   This file is licenced under the LGPLv2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/modprobe.conf and /etc/modprobe.d/*. See <filter>.
*)

module Modprobe =
autoload xfm

(************************************************************************
 * Group:                 USEFUL PRIMITIVES
 *************************************************************************)

(* View: comment *)
let comment = Util.comment

(* View: empty *)
let empty = Util.empty

(* View: sep_space *)
let sep_space = del /([ \t]|(\\\\\n))+/ " "

(* View: sto_no_spaces *)
let sto_no_spaces = store /[^# \t\n\\\\]+/

(* View: sto_to_eol *)
let sto_to_eol = store /[^# \t\n\\\\][^#\n\\\\]*[^# \t\n\\\\]|[^# \t\n\\\\]/

(* View: alias *)
let alias =
  let modulename = [ label "modulename" . sto_no_spaces ] in
  Build.key_value_line_comment "alias" sep_space
                       (sto_no_spaces . sep_space . modulename)
                       comment

(************************************************************************
 * Group:                 ENTRY TYPES
 *************************************************************************)

(* View: options *)
let options =
  let opt_value = /[^#" \t\n\\\\]+|"[^#"\n\\\\]*"/ in
  let option = [ key Rx.word . (Util.del_str "=" . store opt_value)? ] in
  [ key "options" . sep_space . sto_no_spaces
                  . (sep_space . option)* . Util.comment_or_eol ]

(* View: install_remove *)
let kv_line_command (kw:regexp) =
  let command = [ label "command" . sto_to_eol ] in
  [ key kw . sep_space . sto_no_spaces
                         . sep_space . command . Util.comment_or_eol ]

(* View: blacklist *)
let blacklist = Build.key_value_line_comment "blacklist" sep_space
                       sto_no_spaces
                       comment

(* View: config *)
let config = Build.key_value_line_comment "config" sep_space
                       (store /binary_indexes|yes|no/)
                       comment

(* View: entry *)
let entry = alias
          | options
          | kv_line_command /install|remove/
          | blacklist
          | config

(************************************************************************
 * Group:                 LENS AND FILTER
 *************************************************************************)

(* View: lns *)
let lns = (comment|empty|entry)*

(* View: filter *)
let filter = (incl "/etc/modprobe.conf") .
  (incl "/etc/modprobe.d/*").
  (incl "/etc/modprobe.conf.local").
  Util.stdexcl

let xfm = transform lns filter
