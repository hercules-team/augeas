(* Parsing network-scripts/ifcfg-* config files *)
module Ifcfg =
  autoload xfm

  let filter = (incl "/etc/sysconfig/network-scripts/ifcfg-*")
      . Util.stdexcl

  let xfm = transform Shellvars.lns filter

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
