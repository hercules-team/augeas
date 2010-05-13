(*
Module: Modules_conf
  Parses /etc/modules.conf and /etc/conf.modules

  Based on the similar Modprobe lens

  Not all directives currently listed in modules.conf(5) are currently
  supported.
*)
module Modules_conf =
autoload xfm

let comment = Util.comment
let empty = Util.empty
let eol = Util.eol | Util.comment

(* Basic file structure is the same as modprobe.conf *)
let cmd (n:regexp) = Modprobe.cmd n
let token_to_eol = Modprobe.token_to_eol

let path = [ key "path" . del "=" "=" . token_to_eol . eol ]
let keep = [ key "keep" . eol ]
let probeall = [ cmd "probeall" . token_to_eol . eol ]

let entry =
    Modprobe.alias
  | Modprobe.options
  | Modprobe.include
  | Modprobe.cmd_token_to_eol /install|pre-install|post-install/
  | Modprobe.cmd_token_to_eol /remove|pre-remove|post-remove/
  | keep
  | path
  | probeall
  

let lns = (comment|empty|entry)*

let filter = (incl "/etc/modules.conf") .
  (incl "/etc/conf.modules").
  Util.stdexcl

let xfm = transform lns filter
