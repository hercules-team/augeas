(*
Module: Rx
   Generic regexps to build lenses

Author: Raphael Pinson <raphink@gmail.com>

About: License
  This file is licensed under the LGPLv2+, like the rest of Augeas.
*)


module Rx =

(* Spaces *)
let space     = /[ \t]+/
let opt_space = /[ \t]*/
let space_in  = /[^ \t\n].*[^ \t\n]|[^ \t\n]/
let no_spaces = /[^ \t\n]+/

(* Generic fields *)
let word      = /[A-Za-z0-9_.-]+/
let integer   = /[0-9]+/
let decimal   = /[0-9]+([.,][0-9]+)?/

(* A filesystem path *)
let fspath    = /[^ \t\n]+/

(* All but... *)
(* Anything but a space, a comma or a comment sign *)
let neg1      = /[^,# \n\t]+/


(*
 * IPs
 * Cf. http://blog.mes-stats.fr/2008/10/09/regex-ipv4-et-ipv6/ (in fr)
 *)
let ipv4 =
  let dot     = "." in
  let digits  = /(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)/ in
    digits . dot . digits . dot . digits . dot . digits

let ipv6 =
  /(([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})/
  | /(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})/
  | /(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})/
  | /(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})/
  | /(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})/
  | /(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})/
  | (    /([0-9A-Fa-f]{1,4}:){6}/
           . /((((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2})))\.){3}/
           . /(((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2})))/
    )
  | (    /([0-9A-Fa-f]{1,4}:){0,5}:/
           . /((((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2})))\.){3}/
           . /(((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2})))/
    )
  | (    /::([0-9A-Fa-f]{1,4}:){0,5}/
           . /((((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2})))\.){3}/
           . /(((25[0-5])|(1[0-9]{2})|(2[0-4][0-9])|([0-9]{1,2})))/
    )
  | (    /[0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}/
           . /[0-9A-Fa-f]{1,4}/
    )
  | /(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})/
  | /(([0-9A-Fa-f]{1,4}:){1,7}:)/

let ip        = ipv4 | ipv6

(*
 * A Linux device name like eth0 or i2c-0. Might still be too restrictive
 *)

let device_name = /[a-zA-Z0-9_?.+:!-]+/

(*
 * Variable: email_addr
 *    To be refined
 *)
let email_addr = /[A-Za-z0-9_\+\.-]+@[A-Za-z0-9_\.-]+/

