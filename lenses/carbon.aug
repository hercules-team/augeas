(*
Module: Carbon
  Parses Carbon's configuration files
  NB: whitelist.conf and blacklist.conf use a different syntax. This lens
  doesn't support them.

Author: Marc Fournier <marc.fournier@camptocamp.com>

About: License
  This file is licensed under the LGPL v2+, like the rest of Augeas.
*)
module Carbon =
autoload xfm

let comment = IniFile.comment "#" "#"
let sep     = IniFile.sep "=" "="

let entry   = IniFile.entry IniFile.entry_re sep comment
let title   = IniFile.title IniFile.record_re
let record  = IniFile.record title entry

let lns     = IniFile.lns record comment

let filter  = incl "/etc/carbon/carbon.conf"
            . incl "/etc/carbon/relay-rules.conf"
            . incl "/etc/carbon/rewrite-rules.conf"
            . incl "/etc/carbon/storage-aggregation.conf"
            . incl "/etc/carbon/storage-schemas.conf"

let xfm     = transform lns filter
