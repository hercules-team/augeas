(* Lens for Linux syntax of NFS exports(5) *)
module Exports =
  autoload xfm

  let client_re = /[a-zA-Z0-9\-\.@\*\?\/]+/

  let eol = del /[ \t]*\n/ "\n"
  
  let option = [ label "option" . store /[^,)]+/ ]

  let client = [ label "client" . store client_re .
                    ( Util.del_str "(" . 
                        option .
                        ( Util.del_str "," . option ) * .
                      Util.del_str ")" )? ]

  let entry = [ label "dir" . store /\/[^ \t]*/ .
                Util.del_ws_spc .
                client . (Util.del_ws_spc . client)* . eol ]

  let lns = (Hosts.empty | Hosts.comment | entry)*

  let xfm = transform lns (incl "/etc/exports")
