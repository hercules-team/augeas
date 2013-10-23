(*
Module: Dhcpd
  BIND dhcp 3 server configuration module for Augeas

Author: Francis Giraldeau <francis.giraldeau@usherbrooke.ca>

About: Reference
  Reference: manual of dhcpd.conf and dhcp-eval
  Follow dhclient module for tree structure

About: License
    This file is licensed under the GPL.

About: Lens Usage
  Sample usage of this lens in augtool

  Directive without argument.
  Set this dhcpd server authoritative on the domain.
  > clear /files/etc/dhcp3/dhcpd.conf/authoritative

  Directives with integer or string argument.
  Set max-lease-time to one hour:
  > set /files/etc/dhcp3/dhcpd.conf/max-lease-time 3600

  Options are declared as a list, even for single values.
  Set the domain of the network:
  > set /files/etc/dhcp3/dhcpd.conf/option/domain-name/arg example.org
  Set two name server:
  > set /files/etc/dhcp3/dhcpd.conf/option/domain-name-servers/arg[1] foo.example.org
  > set /files/etc/dhcp3/dhcpd.conf/option/domain-name-servers/arg[2] bar.example.org

  Create the subnet 172.16.0.1 with 10 addresses:
  > clear /files/etc/dhcp3/dhcpd.conf/subnet[last() + 1]
  > set /files/etc/dhcp3/dhcpd.conf/subnet[last()]/network 172.16.0.0
  > set /files/etc/dhcp3/dhcpd.conf/subnet[last()]/netmask 255.255.255.0
  > set /files/etc/dhcp3/dhcpd.conf/subnet[last()]/range/from 172.16.0.10
  > set /files/etc/dhcp3/dhcpd.conf/subnet[last()]/range/to 172.16.0.20

  Create a new group "foo" with one static host. Nodes type and address are ordered.
  > ins group after /files/etc/dhcp3/dhcpd.conf/subnet[network='172.16.0.0']/*[last()]
  > set /files/etc/dhcp3/dhcpd.conf/subnet[network='172.16.0.0']/group[last()]/host foo
  > set /files/etc/dhcp3/dhcpd.conf/subnet[network='172.16.0.0']/group[host='foo']/host/hardware/type "ethernet"
  > set /files/etc/dhcp3/dhcpd.conf/subnet[network='172.16.0.0']/group[host='foo']/host/hardware/address "00:00:00:aa:bb:cc"
  > set /files/etc/dhcp3/dhcpd.conf/subnet[network='172.16.0.0']/group[host='foo']/host/fixed-address 172.16.0.100

About: Configuration files
  This lens applies to /etc/dhcpd3/dhcpd.conf. See <filter>.
*)

module Dhcpd =

autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)
let dels (s:string)   = del s s
let eol               = Util.eol
let comment           = Util.comment
let empty             = Util.empty
let indent            = Util.indent
let eos               = comment?

(* Define separators *)
let sep_spc           = del /[ \t]+/ " "
let sep_osp           = del /[ \t]*/ ""
let sep_scl           = del /[ \t]*;([ \t]*\n)*/ ";\n"
let sep_obr           = del /[ \t]*\{([ \t]*\n)*/ " {\n"
let sep_cbr           = del /[ \t]*\}([ \t]*\n)*/ "}\n"
let sep_com           = del /[ \t\n]*,[ \t\n]*/ ", "
let sep_slh           = del "\/" "/"
let sep_col           = del ":" ":"
let sep_eq            = del /[ \t]*=[ \t]*/ "="
let scl               = del ";" ";"

(* Define basic types *)
let word              = /[A-Za-z0-9_.-]+(\[[0-9]+\])?/
let ip                = Rx.ipv4

(* Define fields *)

(* adapted from sysconfig.aug *)
  (* Chars allowed in a bare string *)
  let bchar = /[^ \t\n"'\\{}#,()\/]|\\\\./
  let qchar = /["']/  (* " *)

  (* We split the handling of right hand sides into a few cases:
   *   bare  - strings that contain no spaces, optionally enclosed in
   *           single or double quotes
   *   dquot - strings that contain at least one space, apostrophe or slash
   *           which must be enclosed in double quotes
   *   squot - strings that contain an unescaped double quote
   *)
  let bare = del qchar? "" . store (bchar+) . del qchar? ""
  let quote = Quote.do_quote (store (bchar* . /[ \t'\/]/ . bchar*)+)
  let dquote = Quote.do_dquote (store (bchar+))

let sto_to_spc        = store /[^\\#,;\{\}" \t\n]+|"[^\\#"\n]+"/
let sto_to_scl        = store /[^ \t;][^;\n=]+[^ \t;]|[^ \t;=]+/

let sto_number        = store /[0-9][0-9]*/

(************************************************************************
 *                         NO ARG STATEMENTS
 *************************************************************************)

let stmt_noarg_re     =   "authoritative"
                        | "primary"
                        | "secondary"

let stmt_noarg        = [ indent
                        . key stmt_noarg_re
                        . sep_scl
                        . eos ]

(************************************************************************
 *                         INT ARG STATEMENTS
 *************************************************************************)

let stmt_integer_re   = "default-lease-time"
                      | "max-lease-time"
                      | "min-lease-time"
                      | /lease[ ]+limit/
                      | "port"
                      | /peer[ ]+port/
                      | "max-response-delay"
                      | "max-unacked-updates"
                      | "mclt"
                      | "split"
                      | /load[ ]+balance[ ]+max[ ]+seconds/
                      | "max-lease-misbalance"
                      | "max-lease-ownership"
                      | "min-balance"
                      | "max-balance"
                      | "adaptive-lease-time-threshold"
                      | "dynamic-bootp-lease-length"
                      | "local-port"
                      | "min-sec"
                      | "omapi-port"
                      | "ping-timeout"
                      | "remote-port"

let stmt_integer      = [ indent
                        . key stmt_integer_re
                        . sep_spc
                        . sto_number
                        . sep_scl
                        . eos ]

(************************************************************************
 *                         STRING ARG STATEMENTS
 *************************************************************************)

let stmt_string_re    = "ddns-update-style"
                      | "ddns-updates"
                      | "ddns-hostname"
                      | "ddns-domainname"
                      | "ddns-rev-domainname"
                      | "log-facility"
                      | "server-name"
                      | "fixed-address"
                      | /failover[ ]+peer/
                      | "use-host-decl-names"
                      | "next-server"
                      | "address"
                      | /peer[ ]+address/
                      | "type"
                      | "file"
                      | "algorithm"
                      | "secret"
                      | "key"
                      | "include"
                      | "hba"
                      | "boot-unknown-clients"
                      | "db-time-format"
                      | "do-forward-updates"
                      | "dynamic-bootp-lease-cutoff"
                      | "get-lease-hostnames"
                      | "infinite-is-reserved"
                      | "lease-file-name"
                      | "local-address"
                      | "one-lease-per-client"
                      | "pid-file-name"
                      | "ping-check"
                      | "server-identifier"
                      | "site-option-space"
                      | "stash-agent-options"
                      | "update-conflict-detection"
                      | "update-optimization"
                      | "update-static-leases"
                      | "use-host-decl-names"
                      | "use-lease-addr-for-default-route"
                      | "vendor-option-space"
                      | "primary"

let stmt_string_tpl (kw:regexp) (l:lens) = [ indent
                        . key kw
                        . sep_spc
                        . l
                        . sep_scl
                        . eos ]

let stmt_string  = stmt_string_tpl stmt_string_re bare
                 | stmt_string_tpl stmt_string_re quote
                 | stmt_string_tpl "filename" dquote

(************************************************************************
 *                         RANGE STATEMENTS
 *************************************************************************)

let stmt_range        = [ indent
                        . key "range"
                        . sep_spc
                        . [ label "flag" . store /dynamic-bootp/ . sep_spc ]?
                        . [ label "from" . store ip . sep_spc ]?
                        . [ label "to" . store ip ]
                        . sep_scl
                        . eos ]

(************************************************************************
 *                         HARDWARE STATEMENTS
 *************************************************************************)

let stmt_hardware     = [ indent
                        . key "hardware"
                        . sep_spc
                        . [ label "type" . store /ethernet|tokenring/ ]
                        . sep_spc
                        . [ label "address" . store /[a-fA-F0-9:-]+/ ]
                        . sep_scl
                        . eos ]

(************************************************************************
 *                         OPTION STATEMENTS
 *************************************************************************)
(* The general case is considering options as a list *)

let stmt_option_code  = [ label "label" . store word . sep_spc ]
                        . [ key "code" . sep_spc . store word ]
                        . sep_eq
                        . [ label "type" . store word ]


let stmt_option_list  = ([ label "arg" . bare ] | [ label "arg" . quote ])
                        . ( sep_com . ([ label "arg" . bare ] | [ label "arg" . quote ]))*

let stmt_option_basic = [ key word . sep_spc . stmt_option_list ]
let stmt_option_extra = [ key word . sep_spc . store /true|false/ . sep_spc . stmt_option_list ]

let stmt_option_body = stmt_option_basic | stmt_option_extra

let stmt_option1  = [ indent
                        . key "option"
                        . sep_spc
                        . stmt_option_body
                        . sep_scl
                        . eos ]

let stmt_option2  = [ indent
                        . dels "option" . label "rfc-code"
                        . sep_spc
                        . stmt_option_code
                        . sep_scl
                        . eos ]

let stmt_option = stmt_option1 | stmt_option2

(************************************************************************
 *                         SUBCLASS STATEMENTS
 *************************************************************************)
(* this statement is not well documented in the manual dhcpd.conf
   we support basic use case *)

let stmt_subclass = [ indent . key "subclass" . sep_spc .
                      ([ label "name" . quote ]|
                       [ label "name" . bare ]) . sep_spc .
                       [ label "value" . bare ] . sep_scl . eos ]

(************************************************************************
 *                         ALLOW/DENY STATEMENTS
 *************************************************************************)
(* We have to use special key for allow/deny members of
  to avoid ambiguity in the put direction *)

let allow_deny_re     = "unknown-clients"
                      | "known-clients"
                      | /all[ ]+clients/
                      | /dynamic[ ]+bootp[ ]+clients/
                      | /authenticated[ ]+clients/
                      | /unauthenticated[ ]+clients/
                      | "bootp"
                      | "booting"
                      | "duplicates"
                      | "declines"
                      | "client-updates"
                      | "leasequery"

let stmt_secu_re      = "allow"
                      | "deny"

let del_allow = del /allow[ ]+members[ ]+of/ "allow members of"
let del_deny  = del /deny[ \t]+members[ \t]+of/ "deny members of"

let stmt_secu_tpl (l:lens) (s:string) =
                  [ indent . l . sep_spc . label s . bare . sep_scl . eos ] |
                  [ indent . l . sep_spc . label s . quote . sep_scl . eos ]

let stmt_secu         = [ indent . key stmt_secu_re . sep_spc .
                          store allow_deny_re . sep_scl . eos ] |
                        stmt_secu_tpl del_allow "allow-members-of" |
                        stmt_secu_tpl del_deny "deny-members-of"

(************************************************************************
 *                         MATCH STATEMENTS
 *************************************************************************)

let sto_fct = store (word . /[ \t]*\([^)]*\)/)
let sto_option = store (/option[ ]+/ . word)
let sto_com = /[^ \t\n,\(\)][^,\(\)]*[^ \t\n,\(\)]|[^ \t\n,\(\)]+/ | word . /[ \t]*\([^)]*\)/
let fct_re = "substring" | "binary-to-ascii"

let fct_args = [ label "args" . dels "(" . sep_osp .
                 ([ label "arg" . store sto_com ] . [ label "arg" . sep_com . store sto_com ]+) .
                        sep_osp . dels ")" ]

let stmt_match_if = [ dels "if" . sep_spc . store fct_re . sep_osp . label "function" . fct_args ] .
                      sep_eq . ([ label "value" . bare ]|[ label "value" . quote ])

let stmt_match_pfv = [ label "function" . store "pick-first-value" . sep_spc .
                       dels "(" . sep_osp .
                       [ label "args" .
                         [ label "arg" . store sto_com ] .
                         [ sep_com . label "arg" . store sto_com ]+ ] .
                       dels ")" ]

let stmt_match_tpl (l:lens) = [ indent . key "match" . sep_spc . l . sep_scl . eos ]

let stmt_match = stmt_match_tpl (stmt_match_if | stmt_match_pfv )

(************************************************************************
 *                         BLOCK STATEMENTS
 *************************************************************************)
(* Blocks doesn't support comments at the end of the closing bracket *)

let stmt_entry        =   stmt_secu
                        | stmt_option
                        | stmt_hardware
                        | stmt_range
                        | stmt_string
                        | stmt_integer
                        | stmt_noarg
                        | stmt_match
                        | stmt_subclass
                        | empty
                        | comment

let stmt_block_noarg_re = "pool"
                        | "group"
                        | "allow-update"

let stmt_block_noarg (body:lens)
                        = [ indent
                        . key stmt_block_noarg_re
                        . sep_obr
                        . body*
                        . sep_cbr ]

let stmt_block_arg_re = "host"
                      | "class"
                      | "shared-network"
                      | /failover[ ]+peer/
                      | "zone"
                      | "key"

let stmt_block_arg (body:lens)
                      = [ indent
                        . key stmt_block_arg_re
                        . sep_spc
                        . sto_to_spc
                        . sep_obr
                        . body*
                        . sep_cbr ]

let stmt_block_subnet (body:lens)
                      = [ indent
                        . key "subnet"
                        . sep_spc
                        . [ label "network" . store ip ]
                        . sep_spc
                        . [ key "netmask" . sep_spc . store ip ]
                        . sep_obr
                        . body*
                        . sep_cbr ]

let conditional (body:lens) =
     let condition         = /[^{ \r\t\n][^{\n]*[^{ \r\t\n]|[^{ \t\n\r]/
  in let elsif = [ indent
                 . Build.xchgs "elsif" "@elsif"
                 . sep_spc
                 . store condition
                 . sep_obr
                 . body*
                 . sep_cbr ]
  in let else = [  indent
                 . Build.xchgs "else" "@else"
                 . sep_obr
                 . body*
                 . sep_cbr ]
  in [ indent
     . Build.xchgs "if" "@if"
     . sep_spc
     . store condition
     . sep_obr
     . body*
     . sep_cbr
     . elsif*
     . else? ]


let all_block (body:lens) =
    let lns1 = stmt_block_subnet body in
    let lns2 = stmt_block_arg body in
    let lns3 = stmt_block_noarg body in
    let lns4 = conditional body in
    (lns1 | lns2 | lns3 | lns4 | stmt_entry)

let rec lns_staging = stmt_entry|all_block lns_staging
let lns = (lns_staging)*

let filter = incl "/etc/dhcp3/dhcpd.conf"
           . incl "/etc/dhcp/dhcpd.conf"
           . incl "/etc/dhcpd.conf"

let xfm = transform lns filter
