(* Generic lens for shell-script config files like the ones found *)
(* in /etc/sysconfig, where a string needs to be split into       *)
(* single words.                                                  *)
module Shellvars_list =
  autoload xfm

  let eol = Util.eol

  let key_re = /[A-Za-z0-9_]+/
  let eq      = Util.del_str "="
  let comment = Util.comment
  let empty   = Util.empty
  let indent  = Util.indent

  let sqword = /[^ '\t\n]+/
  let dqword = /([^ "\\\t\n]|\\\\.)+/
  let uqword = /([^ "'\\\t\n]|\\\\.)+/

  (* lists values of the form ...  val1 val2 val3  ... *)
  let list(word:regexp) =
    let list_value = store word in
      indent .
      [ label "value" . list_value ] .
      [ del /[ \t\n]+/ " "  . label "value" . list_value ]* . indent


  (* handle single quoted lists *)
  let squote_arr = [ label "quote" . store /'/ ] . (list sqword)? . del /'/ "'"

  (* similarly handle double qouted lists *)
  let dquote_arr = [ label "quote" . store /"/ ] . (list dqword)? . del /"/ "\""

  (* handle unquoted single value *)
  let unquot_val = [ label "quote" . store "" ] . [label "value"  . store uqword+]?


  (* lens for key value pairs *)
  let kv = [ key key_re . eq . ( squote_arr | dquote_arr | unquot_val ) .  eol ]

  let lns = ( comment | empty | kv )*

  let sc_incl (n:string) = (incl ("/etc/sysconfig/" . n))
  let filter_sysconfig =
      sc_incl "bootloader" .
      sc_incl "kernel"

  let filter = filter_sysconfig
             . Util.stdexcl

  let xfm = transform lns filter

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
