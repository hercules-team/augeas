(* Parsing network-scripts/ifcfg-* config files *)
module Ifcfg =
  autoload xfm

  let eol = Util.del_str "\n"

  let key_re = /[][A-Z0-9_]+/
  let eq = Util.del_str "="
  let value = /[^\n]*/

  let comment = [ del /(#.*)?[ \t]*\n/ "# \n" ]

  let kv = [ key key_re . eq . store value . eol ]

  let lns = (comment | kv) *

  let filter = (incl "/etc/sysconfig/network-scripts/ifcfg-*") 
      . Util.stdexcl

  let xfm = transform lns filter

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
