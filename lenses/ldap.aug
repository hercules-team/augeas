(* Ldap module for Augeas
 Author: Free Ekanayaka <free@64studio.com>
*)

module Ldap =
  autoload xfm

  let filter = Util.stdexcl .
      incl "/etc/ldap.conf" .
      incl "/etc/ldap/ldap.conf"

  let lns = Spacevars.lns

  let xfm = transform lns filter
