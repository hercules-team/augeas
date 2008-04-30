(* Generic lens for shell-script config files like the ones found *)
(* in /etc/sysconfig                                              *)
module Shellvars =

  let eol = Util.del_str "\n"

  let key_re = /[][A-Z0-9_]+/
  let eq = Util.del_str "="
  let value = /[^\n]*/

  let comment = [ del /(#.*)?[ \t]*\n/ "# \n" ]

  let kv = [ key key_re . eq . store value . eol ]

  let lns = (comment | kv) *

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
