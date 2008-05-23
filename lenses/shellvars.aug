(* Generic lens for shell-script config files like the ones found *)
(* in /etc/sysconfig                                              *)
module Shellvars =

  let eol = Util.del_str "\n"

  let key_re = /[][A-Za-z0-9_]+/
  let eq = Util.del_str "="
  let value = /[^\n]*/

  let comment = [ del /(#.*)?[ \t]*\n/ "# \n" ]

  let kv = [ key key_re . eq . store value . eol ]

  let source = 
    [ 
      del /\.|source/ "." . label ".source" . 
      Util.del_ws_spc . store /[^= \t\n]+/ . eol 
    ]

  let lns = (comment | source | kv) *

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
