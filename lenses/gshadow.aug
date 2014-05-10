(* Gshadow module for Augeas

 Original Author: Free Ekanayaka <free@64studio.com>

   see group.aug

 Author: Lorenzo M. Catucci <catucci@ccd.uniroma2.it>

 About: Reference
   man 5 gshadow

 About: License
   This file is licensed under the LGPL v2+, like the rest of Augeas.

*)

module Gshadow =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol        = Util.eol
let comment    = Util.comment
let empty      = Util.empty

let colon      = Sep.colon
let comma      = Sep.comma

let sto_to_spc = store Rx.space_in

let word    = Rx.word
let password = /[A-Za-z0-9_.!*-]*/
let integer = Rx.integer

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let member       = [ label "member" . store word ]
let member_list  = Build.opt_list member comma

let admin      = [ label "admin" . store word ]
let admin_list = Build.opt_list admin comma

let params     = [ label "password"  . store password  . colon ]
		 .  admin_list?     . colon
                 .  member_list?

let entry      = Build.key_value_line word colon params

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = (comment|empty|entry) *

let filter
               = incl "/etc/gshadow"
               . Util.stdexcl

let xfm        = transform lns filter
