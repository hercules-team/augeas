(* Variation of the Shellvars lens                                     *)
(* Supports only what's needed to handle sysconfig files               *)
(* Modified to strip quotes. In the put direction, add double quotes   *)
(* around values that need them                                        *)
(* To keep things simple, we also do not support shell variable arrays *)
module Sysconfig =
  let eol = Util.eol

  let key_re = /[A-Za-z0-9_]+(\[[0-9]+\])?/ - "unset" - "export"
  let eq = Util.del_str "="
  let comment = Util.comment
  let empty   = Util.empty
  let xchgs   = Build.xchgs
  let dels    = Util.del_str

  let nothing = del /(""|'')?/ "" . value ""

  (* Chars allowed in a bare string *)
  let bchar = /[^ \t\n\"'\\]|\\\\./
  let qchar = /["']/  (* " *)

  (* We split the handling of right hand sides into a few cases:
   *   bare  - strings that contain no spaces, optionally enclosed in
   *           single or double quotes
   *   dquot - strings that contain at least one space or apostrophe,
   *           which must be enclosed in double quotes
   *   squot - strings that contain an unescaped double quote
   *)
  let bare = del qchar? "" . store (bchar+) . del qchar? ""
  let dquot =
    del qchar "\"" . store (bchar* . /[ \t']/ . bchar*)+ . del qchar "\""
  let squot =
    dels "'" . store ((bchar|/[ \t]/)* . "\"" . (bchar|/[ \t]/)*)+ . dels "'"

  let export = [ key "export" . Util.del_ws_spc ]
  let kv (value:lens) = [ export? . key key_re . eq . value . eol ]
  let assign = kv nothing | kv bare | kv dquot | kv squot

  let var_action (name:string) =
    [ xchgs name ("@" . name) . Util.del_ws_spc . store key_re . eol ]

  let unset = var_action "unset"
  let bare_export = var_action "export"

  let source =
    [
      del /\.|source/ "." . label ".source" .
      Util.del_ws_spc . store /[^= \t\n]+/ . eol
    ]

  let lns = (comment | empty | source | assign | unset | bare_export) *

(*
  Examples:

  abc   -> abc -> abc
  "abc" -> abc -> abc
  "a b" -> a b -> "a b"
  'a"b' -> a"b -> 'a"b'
*)
