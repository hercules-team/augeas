(*
 Module: Passwd
 Parses /etc/passwd

 Author: Free Ekanayaka <free@64studio.com>

 About: Reference
        - man 5 passwd
        - man 3 getpwnam

 Each line in the unix passwd file represents a single user record, whose
 colon-separated attributes correspond to the members of the passwd struct

*)

module Passwd =

   autoload xfm

(************************************************************************
 * Group:                    USEFUL PRIMITIVES
 *************************************************************************)

(* Group: Comments and empty lines *)

let eol        = Util.eol
let comment    = Util.comment
let empty      = Util.empty
let dels       = Util.del_str

let word       = /[_.A-Za-z0-9][-\@_.A-Za-z0-9]*\$?/
let integer    = Rx.integer

let colon      = Sep.colon

let sto_to_eol = store Rx.space_in
let sto_to_col = store /[^:\r\n]+/

(************************************************************************
 * Group:                        ENTRIES
 *************************************************************************)

(* View: password
        pw_passwd *)
let password  = [ label "password" . sto_to_col?   . colon ]

(* View: uid
        pw_uid *)
let uid       = [ label "uid"      . store integer . colon ]

(* View: gid
        pw_gid *)
let gid       = [ label "gid"      . store integer . colon ]

(* View: name
        pw_gecos; the user's full name *)
let name      = [ label "name"     . sto_to_col? . colon ]

(* View: home
        pw_dir *)
let home      = [ label "home"     . sto_to_col?   . colon ]

(* View: shell
        pw_shell *)
let shell     = [ label "shell"    . sto_to_eol? ]

(* View: entry
        struct passwd *)
let entry     = [ key word
                . colon
                . password
                . uid
                . gid
                . name
                . home
                . shell
                . eol ]

(* A NIS entry has nothing bar the +@:::::: bits. *)
let nisentry =
  let overrides =
        colon
      . [ label "password" . sto_to_col ]?   . colon
      . [ label "uid"      . store integer ]? . colon
      . [ label "gid"      . store integer ]? . colon
      . [ label "name"     . sto_to_col ]?   . colon
      . [ label "home"     . sto_to_col ]?  . colon
      . [ label "shell"    . sto_to_eol ]? in
  [ dels "+@" . label "@nis" . store word . overrides . eol ]

let nisuserplus =
  let overrides =
        colon
      . [ label "password" . store word ]?    . colon
      . [ label "uid"      . store integer ]? . colon
      . [ label "gid"      . store integer ]? . colon
      . [ label "name"     . sto_to_col ]?    . colon
      . [ label "home"     . sto_to_col ]?    . colon
      . [ label "shell"    . sto_to_eol ]? in
  [ dels "+" . label "@+nisuser" . store word . overrides . eol ]

let nisuserminus =
  let overrides =
        colon
      . [ label "password" . store word ]?    . colon
      . [ label "uid"      . store integer ]? . colon
      . [ label "gid"      . store integer ]? . colon
      . [ label "name"     . sto_to_col ]?    . colon
      . [ label "home"     . sto_to_col ]?    . colon
      . [ label "shell"    . sto_to_eol ]? in
  [ dels "-" . label "@-nisuser" . store word . overrides . eol ]

let nisdefault =
  let overrides =
        colon
      . [ label "password" . store word?    . colon ]
      . [ label "uid"      . store integer? . colon ]
      . [ label "gid"      . store integer? . colon ]
      . [ label "name"     . sto_to_col?    . colon ]
      . [ label "home"     . sto_to_col?    . colon ]
      . [ label "shell"    . sto_to_eol? ] in
  [ dels "+" . label "@nisdefault" . overrides? . eol ]

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = (comment|empty|entry|nisentry|nisdefault|nisuserplus|nisuserminus) *

let filter     = incl "/etc/passwd"

let xfm        = transform lns filter
