(*
Module: MCollective
  Parses MCollective's {client,server}.cfg configuration files

Author: Marc Fournier <marc.fournier@camptocamp.com>

About: License
  This file is licensed under the LGPL v2+, like the rest of Augeas.
*)

module MCollective =
autoload xfm

let lns = Simplevars.lns

let filter = incl "/etc/mcollective/client.cfg"
           . incl "/etc/mcollective/server.cfg"

let xfm = transform lns filter
