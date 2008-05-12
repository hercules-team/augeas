(* /etc/sshd/sshd_config *)
module Sshd =
   autoload xfm

   let eol = del /[ \t]*\n/ "\n"

   let sep = Util.del_ws_spc

   let key_re = /[A-Za-z0-9]+/ 
         - /MACs|Match|AcceptEnv|(Allow|Deny)(Groups|Users)/

   let comment = [ del /(#.*|[ \t]*)\n/ "\n" ]

   let array_entry (k:string) =
     let value = store /[^ \t\n]+/ in
     [ key k . [ sep . seq k . value]* . eol ]

   let other_entry =
     let value = store /[^ \t\n]+([ \t]+[^ \t\n]+)*/ in
     [ key key_re . sep . value . eol ]

   let accept_env = array_entry "AcceptEnv"

   let allow_groups = array_entry "AllowGroups"
   let allow_users = array_entry "AllowUsers"
   let deny_groups = array_entry "DenyGroups"
   let deny_users = array_entry "DenyUsers"

   let macs =
     let mac_value = store /[^, \t\n]+/ in
     [ key "MACs" . sep .
         [ seq "macs" . mac_value ] .
         ([ seq "macs" . Util.del_str "," . mac_value])* .
         eol ]

   let match =
     let value = store /[^ \t\n]+([ \t]+[^ \t\n]+)*/ in
     [ key "Match" . sep .
         [ seq "match" .
             [ label "cond" . value . eol ] .
             (sep . other_entry) *
         ]
     ]

  let lns = (comment | accept_env | allow_groups | allow_users
          | deny_groups | deny_users | macs | match | other_entry ) *

  let xfm = transform lns (incl "/etc/ssh/sshd_config")

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
