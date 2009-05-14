(*
Module: Modprobe
  Parses /etc/modprobe.conf and /etc/modprobe.d/*
*)
module Modprobe =
autoload xfm

let comment = Util.comment
let empty = Util.empty
let eol = Util.eol | Util.comment

(* modprobe.conf allows continuing a line by ending it with backslash +
   newline; the backslash + newline token is suppressed We handle an
   approximation of that by classifying backslash + newline as a
   separator.
*)

(* A separator is either whitespace or \ followed by newline *)
let sep_ch = /[ \t]|\\\\\n/
(* Anything that's not a separator is part of a token *)
let tok_ch = /[^ \t\n#\\]|\\\\[^ \t\n]/

let spc = del sep_ch+ " "
let token = store tok_ch+
let indent = Util.del_opt_ws ""

let cmd (n:regexp) = key n . spc
let arg (n:string) = [ label n . token ]
let token_to_eol = store (tok_ch . /([^#\n\\]|\\\\\n)*/ . tok_ch | tok_ch)

let options =
  let opt_ch = /[A-Za-z0-9_]/ in
  let option = [ spc . key opt_ch+ . (del /=/ "=" . token)? ] in
    [ cmd "options" . token . option* . eol ]

let entry = [ cmd "alias" . token . spc . arg "modulename" . eol ]
  | [ cmd "include" . token . eol ]
  | options
  | [ cmd /install|remove/ . token_to_eol . eol ]
  | [ cmd "blacklist" . token . eol ]
  | [ cmd "config" . store /binary_indexes|yes|no/ ]

let lns = (comment|empty|entry)*

let filter = (incl "/etc/modprobe.conf") .
  (incl "/etc/modprobe.d/*").
  Util.stdexcl

let xfm = transform lns filter
