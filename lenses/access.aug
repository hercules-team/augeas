(* 
Module: Access
  Parses /etc/security/access.conf

Author: Lorenzo Dalrio <lorenzo.dalrio@gmail.com>

About: Reference
  Some examples of valid entries can be found in access.conf or "man access.conf"

About: License
  This file is licensed under the LGPLv2+, like the rest of Augeas.

About: Lens Usage
  Sample usage of this lens in augtool

  * Add a rule to permit login of all users from local sources (tty's, X, cron)
  > set /files/etc/security/access.conf[0] +
  > set /files/etc/security/access.conf[0]/user ALL
  > set /files/etc/security/access.conf[0]/host LOCAL

About: Configuration files
  This lens applies to /etc/security/access.conf. See <filter>.
*)
module Access =
  autoload xfm

(* Group: Comments and empty lines *)
(* Variable: comment *)
let comment   = Util.comment
(* Variable: empty line *)
let empty     = Util.empty
(* Group: Useful primitives *)
(* Variable: colon
 *  this is the standard field separator " : "
 *)
let colon     = Sep.space . Sep.colon . Sep.space


(************************************************************************
 * Group:                     ENTRY LINE
  *************************************************************************)
(* View: access
 * Allow (+) or deny (-) access
 *)
let access    = label "access" . store /[+-]/

(* View: user
 * user can be a username or a group
 *)
let user      = [ label "user" . store Rx.word ]

(* View: host_list
 * host_list can be a single ipaddr/hostname/domain/fqdn or a list of those values
 *)
let host_list = Build.opt_list [ label "host" . store Rx.word ] Sep.space

(* View: entry 
 * A valid entry line
 * Definition:
 *   > entry ::= access ':' user ':' host_list
 *)
let entry     = [ access . colon
                . user . colon
                . host_list . Util.eol ]

(************************************************************************
 * Group:                        LENS & FILTER
  *************************************************************************)
(* View: lns
    The access.conf lens, any amount of
      * <empty> lines
      * <comments>
      * <entry>
*)
let lns       = (comment|empty|entry) *

(* Variable: filter *)
let filter    = incl "/etc/security/access.conf"

let xfm       = transform lns filter
