(*
Module: Nrpe
  Parses nagios-nrpe configuration files.

Author: Marc Fournier <marc.fournier@camptocamp.com>

About: License
  This file is licensed under the LGPLv2+, like the rest of Augeas.
*)

module Nrpe =
  autoload xfm

let nl = del /[ \t]*\n/ "\n"
let eq = del /=/ "="


(* command entries

nrpe.cfg usually has many entries defining commands to run

command[check_foo]=/path/to/nagios/plugin -w 123 -c 456
command[check_bar]=/path/to/another/nagios/plugin --option

*)
let obrkt = del /\[/ "["
let cbrkt = del /\]/ "]"

let command = [ key "command" .
  [ obrkt . key /[^]\/\n]+/ . cbrkt . eq . store /[^=\n]+/ . del /\n/ "\n" ]
]


(* regular entries *)
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

let item = [ key item_re . eq . store /[^=\n\t ]+/ . nl ]


(* include entries

nrpe.cfg can include more than one file or directory of files

include=/path/to/file1.cfg
include=/path/to/file2.cfg
include_dir=/path/to/dir/

*)
let include = [ key "include" .
  [ label "file" . eq . store /[^=\n\t ]+/ . nl ]
]
let include_dir = [ key "include_dir" .
  [ label "dir" . eq . store /[^=\n\t ]+/ . nl ]
]


(* comments

We can't use Util.comment because nrpe only allows comments starting at
beginning of line.

*)
let comment = [ label "#comment" . del /^#[ \t]*/ "# " . store /([^ \t\n].*[^ \t\n]|[^ \t\n])/ . nl ]


(* blank lines and empty comments *)
let empty = Util.empty


(* final lens definition *)
let lns = ( command | item | include | include_dir | comment | empty ) *

let filter = Util.stdexcl .
             incl "/etc/nrpe.cfg" .
             incl "/etc/nagios/nrpe.cfg"

let xfm = transform lns (filter)
