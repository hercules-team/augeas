(* Ldap module for Augeas
 Author: Free Ekanayaka <free@64studio.com>
*)

module Ldap =
  autoload xfm

  (* plain spacevars lens *)
  let entry = Spacevars.entry Spacevars.entry_re
  let lns   = Spacevars.lns entry

  let filter = Util.stdexcl .
      incl "/etc/ldap.conf" .
      incl "/etc/ldap/ldap.conf"

  let xfm = transform lns filter
