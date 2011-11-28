(*
Module: Reprepro_Uploaders
  Parses reprepro's uploaders files

Author: Raphael Pinson <raphink@gmail.com>

About: Reference
  This lens tries to keep as close as possible to `man 1 reprepro` where possible.

About: License
   This file is licenced under the LGPLv2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to reprepro's uploaders files.
*)

module Reprepro_Uploaders =

(* View: condition_kw
   Keywords for <condition> fields *)
let condition_re =
  let contain (kw:regexp) = kw . (Rx.space . "contain")? in
                   "source"
                 | contain "sections"
                 | contain "binaries"
                 | contain "architectures"
                 | "byhand"

(* View: logic_construct
   A logical construction for <condition> and <condition_list> *)
let logic_construct (kw:string) (lns:lens) =
  [ label kw . lns ]
  . [ Sep.space . key kw . Sep.space . lns ]*

(* View: conditionf_field
   A single condition field *)
let condition_field =
  [ key condition_re . Sep.space
  . Util.del_str "'" . store /[^'\n]+/ . Util.del_str "'" ]

(* View: condition *)
let condition =
  logic_construct "or" condition_field

(* View: condition_list
   A list of <condition>s, inspired by Debctrl.dependency_list *)
let condition_list =
  store "*"
  | logic_construct "and" condition

(* View: by_key *)
let by_key =
  let any_key   = [ store "any" . Sep.space
                  . key "key" ] in
  let named_key = [ key "key" . Sep.space
                  . store (Rx.word - "any") ] in
  value "key" . (any_key | named_key)

(* View: by *)
let by =
  [ key "by" . Sep.space
         . ( store ("anybody"|"unsigned")
           | by_key ) ]

(* View: entry *)
let entry =
  [ key "allow" . Sep.space
  . condition_list . Sep.space
  . by . Util.eol ]

(* View: lns
   The lens *)
let lns = (Util.empty|Util.comment|entry)*
