(*
Module: Nrpe
  Parses nagios-nrpe configuration files.

Author: Marc Fournier <marc.fournier@camptocamp.com>

About: License
  This file is licensed under the LGPLv2+, like the rest of Augeas.
*)

module Nrpe =
  autoload xfm


let eol = Util.eol
let eq = Sep.equal

(* View: word *)
let word = /[^=\n\t ]+/


(* View: command
    nrpe.cfg usually has many entries defining commands to run

    > command[check_foo]=/path/to/nagios/plugin -w 123 -c 456
    > command[check_bar]=/path/to/another/nagios/plugin --option
*)
let command =
  let obrkt = del /\[/ "[" in
  let cbrkt = del /\]/ "]" in
    [ key "command" .
    [ obrkt . key /[^]\/\n]+/ . cbrkt . eq
            . store /[^\n]+/ . del /\n/ "\n" ]
    ]


(* View: item_re
     regular entries re *)
let item_re = "server_port"
            | "command_prefix"
            | "server_address"
            | "allowed_hosts"
            | "debug"
            | "nrpe_user"
            | "nrpe_group"
            | "dont_blame_nrpe"
            | "command_timeout"
            | "connection_timeout"
            | "allow_weak_random_seed"
            | "pid_file"
            | "log_facility"

(* View: item
     regular entries *)
let item = [ key item_re . eq . store word . eol ]


(* View: include
    An include entry.

    nrpe.cfg can include more than one file or directory of files

    > include=/path/to/file1.cfg
    > include=/path/to/file2.cfg
*)
let include = [ key "include" .
  [ label "file" . eq . store word . eol ]
]

(* View: include_dir
    > include_dir=/path/to/dir/
*)
let include_dir = [ key "include_dir" .
  [ label "dir" . eq . store word . eol ]
]


(* View: comment
    Nrpe comments must start at beginning of line *)
let comment = Util.comment_generic /#[ \t]*/ "# "

(* blank lines and empty comments *)
let empty = Util.empty

(* View: lns
    The Nrpe lens *)
let lns = ( command | item | include | include_dir | comment | empty ) *

(* View: filter
    File filter *)
let filter = Util.stdexcl .
             incl "/etc/nrpe.cfg" .
             incl "/etc/nagios/nrpe.cfg"

let xfm = transform lns (filter)

