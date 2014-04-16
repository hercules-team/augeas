(*
Module: Redis
  Parses Redis's configuration files

Author: Marc Fournier <marc.fournier@camptocamp.com>

About: Reference
    This lens is based on Redis's default redis.conf

About: Usage Example
(start code)
augtool> set /augeas/load/Redis/incl "/etc/redis/redis.conf"
augtool> set /augeas/load/Redis/lens "Redis.lns"
augtool> load

augtool> get /files/etc/redis/redis.conf/vm-enabled
/files/etc/redis/redis.conf/vm-enabled = no
augtool> print /files/etc/redis/redis.conf/rename-command[1]/
/files/etc/redis/redis.conf/rename-command
/files/etc/redis/redis.conf/rename-command/from = "CONFIG"
/files/etc/redis/redis.conf/rename-command/to = "CONFIG2"

augtool> set /files/etc/redis/redis.conf/activerehashing no
augtool> save
Saved 1 file(s)
augtool> set /files/etc/redis/redis.conf/save[1]/seconds 123
augtool> set /files/etc/redis/redis.conf/save[1]/keys 456
augtool> save
Saved 1 file(s)
(end code)
   The <Test_Redis> file also contains various examples.

About: License
  This file is licensed under the LGPL v2+, like the rest of Augeas.
*)

module Redis =
autoload xfm

let k = Rx.word
let v = /[^ \t\n'"]+/
let comment = Util.comment
let empty = Util.empty
let indent = Util.indent
let eol = Util.eol
let del_ws_spc = Util.del_ws_spc
let dquote = Util.del_str "\""

(* View: standard_entry
A standard entry is a key-value pair, separated by blank space, with optional
blank spaces at line beginning & end. The value part can be optionnaly enclosed
in single or double quotes. Comments at end-of-line ar NOT allowed by
redis-server.
*)
let standard_entry =
     let reserved_k = "save" | "rename-command" | "slaveof"
                    | "client-output-buffer-limit"
  in let entry_noempty = [ indent . key k . del_ws_spc
                         . Quote.do_quote_opt_nil (store v) . eol ]
  in let entry_empty = [ indent . key (k - reserved_k) . del_ws_spc
                         . dquote . store "" . dquote . eol ]
  in entry_noempty | entry_empty

let save = /save/
let seconds = [ label "seconds" . Quote.do_quote_opt_nil (store Rx.integer) ]
let keys = [ label "keys" . Quote.do_quote_opt_nil (store Rx.integer) ]
(* View: save_entry
Entries identified by the "save" keyword can be found more than once. They have
2 mandatory parameters, both integers. The same rules as standard_entry apply
for quoting, comments and whitespaces.
*)
let save_entry = [ indent . key save . del_ws_spc . seconds . del_ws_spc . keys . eol ]

let slaveof = /slaveof/
let ip = [ label "ip" . Quote.do_quote_opt_nil (store Rx.ip) ]
let port = [ label "port" . Quote.do_quote_opt_nil (store Rx.integer) ]
(* View: slaveof_entry
Entries identified by the "slaveof" keyword can be found more than once. They
have 2 mandatory parameters, the 1st one is an IP address, the 2nd one is a
port number. The same rules as standard_entry apply for quoting, comments and
whitespaces.
*)
let slaveof_entry = [ indent . key slaveof . del_ws_spc . ip . del_ws_spc . port . eol ]

let renamecmd = /rename-command/
let from = [ label "from" . Quote.do_quote_opt_nil (store Rx.word) ]
let to = [ label "to" . Quote.do_quote_opt_nil (store Rx.word) ]
(* View: save_entry
Entries identified by the "rename-command" keyword can be found more than once.
They have 2 mandatory parameters, both strings. The same rules as
standard_entry apply for quoting, comments and whitespaces.
*)
let renamecmd_entry = [ indent . key renamecmd . del_ws_spc . from . del_ws_spc . to . eol ]

let cobl_cmd = /client-output-buffer-limit/
let class = [ label "class" . Quote.do_quote_opt_nil (store Rx.word) ]
let hard_limit = [ label "hard_limit" . Quote.do_quote_opt_nil (store Rx.word) ]
let soft_limit = [ label "soft_limit" . Quote.do_quote_opt_nil (store Rx.word) ]
let soft_seconds = [ label "soft_seconds" . Quote.do_quote_opt_nil (store Rx.integer) ]
(* View: client_output_buffer_limit_entry
Entries identified by the "client-output-buffer-limit" keyword can be found
more than once. They have four mandatory paramters, of which the first is a
string, the last one is an integer and the others are either integers or words,
although redis is very liberal and takes "4242yadayadabytes" as a valid limit.
The same rules as standard_entry apply for quoting, comments and whitespaces.
*)
let client_output_buffer_limit_entry =
  [ indent . key cobl_cmd . del_ws_spc . class . del_ws_spc . hard_limit .
    del_ws_spc . soft_limit . del_ws_spc . soft_seconds . eol ]

let entry = standard_entry
          | save_entry
	  | renamecmd_entry
	  | slaveof_entry
	  | client_output_buffer_limit_entry

(* View: lns
The Redis lens
*)
let lns = (comment | empty | entry )*

let filter = incl "/etc/redis/redis.conf"

let xfm = transform lns filter
