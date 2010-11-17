module Iptables =
  autoload xfm

(*
Module: Iptables
   Parse the iptables file format as produced by iptables-save. The
   resulting tree is fairly simple; in particular a rule is simply
   a long list of options/switches and their values (if any)

   This lens should be considered experimental
*)

let comment = Util.comment
let empty = Util.empty
let eol = Util.eol
let spc = Util.del_ws_spc
let dels = Util.del_str

let chain_name = store /[A-Za-z0-9_-]+/
let chain =
  let policy = [ label "policy" . store /ACCEPT|DROP|REJECT|-/ ] in
  let counters_eol = del /[ \t]*(\[[0-9:]+\])?[ \t]*\n/ "\n" in
    [ label "chain" .
        dels ":" . chain_name . spc . policy . counters_eol ]

let param (long:string) (short:string) =
  [ label long .
      spc . del (/--/ . long | /-/ . short) ("-" . short) . spc .
      store /(![ \t]*)?[^ \t\n!-][^ \t\n]*/ ]

(* A negatable parameter, which can either be FTW
     ! --param arg
   or
     --param ! arg
*)
let neg_param (long:string) (short:string) =
  [ label long .
      [ spc . dels "!" . label "not" ]? .
      spc . del (/--/ . long | /-/ . short) ("-" . short) . spc .
      store /(![ \t]*)?[^ \t\n!-][^ \t\n]*/ ]

(* misses --set-counters *)
let ipt_match =
  let any_key = /[a-zA-Z-][a-zA-Z-]+/ -
    /protocol|source|destination|jump|goto|in-interface|out-interface|fragment|match/ in
  let any_val = /([^\" \t\n!-][^ \t\n]*)|\"([^\"\\\n]|\\\\.)*\"/ in
  let any_param =
    [ [ spc . dels "!" . label "not" ]? .
      spc . dels "--" . key any_key . (spc . store any_val)? ] in
    (neg_param "protocol" "p"
    |neg_param "source" "s"
    |neg_param "destination" "d"
    |param "jump" "j"
    |param "goto" "g"
    |neg_param "in-interface" "i"
    |neg_param "out-interface" "o"
    |neg_param "fragment" "f"
    |param "match" "m"
    |any_param)*

let add_rule =
  let chain_action (n:string) (o:string) =
    [ label n .
        del (/--/ . n | o) o .
        spc . chain_name . ipt_match . eol ] in
    chain_action "append" "-A" | chain_action "insert" "-I"

let table = [ del /\*/ "*" . label "table" . store /[a-z]+/ . eol .
                (chain|comment)* . (add_rule . comment*)* .
                dels "COMMIT" . eol ]

let lns = (comment|empty|table)*
let xfm = transform lns (incl "/etc/sysconfig/iptables"
                       . incl "/etc/iptables-save")
