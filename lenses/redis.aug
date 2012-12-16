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

(* View: standard_entry
A standard entry is a key-value pair, separated by blank space, with optional
blank spaces at line beginning & end. The value part can be optionnaly enclosed
in single or double quotes. Comments at end-of-line ar NOT allowed by
redis-server.
*)
let standard_entry =  [ indent . key k . del_ws_spc . Quote.do_quote_opt_nil (store v) . eol ]

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

(* View: lns
The Redis lens
*)
let lns = (comment | empty | standard_entry | save_entry | renamecmd_entry | slaveof_entry )*

let filter = incl "/etc/redis/redis.conf"

let xfm = transform lns filter
