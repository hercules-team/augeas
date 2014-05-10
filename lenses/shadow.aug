(* Shadow module for Augeas

 Original Author: Free Ekanayaka <free@64studio.com>

   see passwd.aug

 Author: Lorenzo M. Catucci <catucci@ccd.uniroma2.it>

 About: Reference
   man 5 shadow

 About: License
   This file is licensed under the LGPL v2+, like the rest of Augeas.

*)

module Shadow =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol        = Util.eol
let comment    = Util.comment
let empty      = Util.empty
let dels       = Util.del_str

let colon      = Sep.colon

let word       = Rx.word
let integer    = Rx.integer

let sto_to_col = Passwd.sto_to_col
let sto_to_eol = Passwd.sto_to_eol

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let entry   = [ key word
                . colon
                . [ label "password"          . sto_to_col?    . colon ]
                . [ label "lastchange_date"   . store integer? . colon ]
                . [ label "minage_days"       . store integer? . colon ]
                . [ label "maxage_days"       . store integer? . colon ]
                . [ label "warn_days"         . store integer? . colon ]
                . [ label "inactive_days"     . store integer? . colon ]
                . [ label "expire_date"       . store integer? . colon ]
                . [ label "flag"              . store integer? ]
                . eol ]

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = (comment|empty|entry) *

let filter
               = incl "/etc/shadow"
               . Util.stdexcl

let xfm        = transform lns filter
