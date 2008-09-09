(* /etc/sshd/sshd_config *)
module Sshd =
   autoload xfm

   let eol = del /[ \t]*\n/ "\n"

   let sep = Util.del_ws_spc

   let key_re = /[A-Za-z0-9]+/ 
         - /MACs|Match|AcceptEnv|Subsystem|(Allow|Deny)(Groups|Users)/

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

   let subsystemvalue = 
     let value = store  /[^ \t\n]+/ in
     [ key /[A-Za-z0-9]+/ . sep . value . eol ]  

   let subsystem = 
     let value = store  /[^ \t\n]+([ \t]+[^ \t\n]+)*/ in
     [ key "Subsystem" .  sep .  subsystemvalue ]  

   let macs =
     let mac_value = store /[^, \t\n]+/ in
     [ key "MACs" . sep .
         [ seq "macs" . mac_value ] .
         ([ seq "macs" . Util.del_str "," . mac_value])* .
         eol ]

   let match_cond = 
     [ label "Condition" . sep . [ key /[A-Za-z0-9]+/ . sep . 
                             store /[^ \t\n]+/ ] ]

   let match_entry = 
     ( comment | (Util.indent . other_entry) )

   let match =
     [ key "Match" . match_cond+ . del / */ "" . del "\n" "\n"
        . [ label "Settings" .  match_entry+ ]
     ]

  let lns = (comment | accept_env | allow_groups | allow_users
          | deny_groups | subsystem | deny_users | macs 
          | other_entry ) * . match*

  let xfm = transform lns (incl "/etc/ssh/sshd_config")

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
