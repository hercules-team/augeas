(* Process /etc/multipath.conf                             *)
(* The lens is based on the multipath.conf(5) man page     *)
module Multipath =

autoload xfm

let comment = Util.comment
let empty = Util.empty
let dels = Util.del_str
let eol = Util.eol

let ws = del /[ \t]+/ " "
let indent = del /[ \t]*/ ""
(* We require that braces are always followed by a newline *)
let obr = del /\{([ \t]*)\n/ "{\n"
let cbr = del /[ \t]*}[ \t]*\n/ "}\n"

let ikey (k:regexp) = indent . key k

let section (n:regexp) (b:lens) =
  [ ikey n . ws . obr . (b|empty|comment)* . cbr ]

let kv (k:regexp) (v:regexp) =
  [ ikey k . ws . store v . eol ]

(* FIXME: it would be much more concise to write                       *)
(* [ key k . ws . (bare | quoted) ]                                    *)
(* but the typechecker trips over that                                 *)
let qstr (k:regexp) =
  let delq = del /['"]/ "\"" in
  let bare = del /["']?/ "" . store /[^"' \t\n]+/ . del /["']?/ "" in
  let quoted = delq . store /.*[ \t].*/ . delq in
  [ ikey k . ws . bare . eol ]
 |[ ikey k . ws . quoted . eol ]

let wwid = kv "wwid" (Rx.word|"*")

(* Settings that can be changed in various places *)
let common_setting =
  kv "path_grouping_policy"
    /failover|multibus|group_by_(serial|prio|node_name)/
 |qstr /(getuid|prio)_callout/
 |qstr /path_(selector|checker)|features/
 |kv "failback" (Rx.integer | /immediate|manual/)
 |kv "rr_weight" /priorities|uniform/
 |kv "no_path_retry" (Rx.integer | /fail|queue/)
 |kv "rr_min_io" Rx.integer

let default_setting =
  kv "polling_interval" Rx.integer
  |kv "udev_dir" Rx.fspath
  |qstr "selector"
  |kv "user_friendly_names" /yes|no/
  |kv "dev_loss_tmo" Rx.integer
  |kv "fast_io_fail_tmo" Rx.integer
  |kv "verbosity" /[0-6]/
  |kv "reassign_maps" /yes|no/
  (* These are not in the manpage but in the example multipath.conf *)
  |kv "prio" Rx.word
  |kv "max_fds" Rx.integer
  (* SUSE extensions *)
  |kv "async_timeout" Rx.integer
  |kv "max_polling_interval" Rx.integer

(* A device subsection *)
let device =
  let setting =
    qstr /vendor|product|product_blacklist|hardware_handler/
   |common_setting
   |default_setting in
  section "device" setting

(* The defaults section *)
let defaults =
  let setting =
    common_setting
   |default_setting
  in section "defaults" setting

(* The blacklist and blacklist_exceptions sections *)
let blacklist =
  let setting =
    wwid
   |qstr "devnode"
   |device in
  section /blacklist(_exceptions)?/ setting

(* A multipath subsection *)
let multipath =
  let setting =
    wwid
   |qstr "alias"
   |common_setting in
  section "multipath" setting

(* The multipaths section *)
let multipaths =
  section "multipaths" multipath

(* The devices section *)
let devices =
  section "devices" device

let lns = (comment|empty|defaults|blacklist|devices|multipaths)*

let xfm = transform lns (incl "/etc/multipath.conf")
