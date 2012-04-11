(*
Module: Kdump
  Parses /etc/kdump.conf

Author: Roman Rakus <rrakus@redhat.com>

About: References
  manual page kdump.conf(5)

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Configuration files
   This lens applies to /etc/kdump.conf. See <filter>.
*)

module Kdump =
  autoload xfm

let empty = Util.empty
let comment = Util.comment
let value_to_eol = store Rx.space_in
let delimiter = Util.del_ws_spc
let eol = Util.eol
let value_to_spc = store Rx.no_spaces
let key_to_space = key /[A-Za-z0-9_.\$-]+/
let eq = Util.del_str "="

let simple_entry (kw:string) = [ key kw . delimiter . value_to_eol . eol ]

(* Got from mount(8) *)
let fs_types = "adfs" | "affs" | "autofs" | "cifs" | "coda" | "coherent"
             | "cramfs" | "debugfs" | "devpts" | "efs" | "ext" | "ext2"
             | "ext3" | "ext4" | "hfs" | "hfsplus" | "hpfs" | "iso9660"
             | "jfs" | "minix" | "msdos" | "ncpfs" | "nfs" | "nfs4" | "ntfs"
             | "proc" | "qnx4" | "ramfs" | "reiserfs" | "romfs" | "squashfs"
             | "smbfs" | "sysv" | "tmpfs" | "ubifs" | "udf" | "ufs" | "umsdos"
             | "usbfs" | "vfat" | "xenix" | "xfs" | "xiafs"

let fs = [ key fs_types . delimiter . value_to_eol . eol ]

let bin = [ seq "bin" . delimiter . value_to_spc ]
let extra_bins = [ key "extra_bins" . (bin)+ . eol ]

let mdl = [ seq "module" . delimiter . value_to_spc ]
let extra_modules = [ key "extra_modules" . (mdl)+ . eol ]


let mdl_key_value = [ delimiter . key_to_space . ( eq . value_to_spc)? ]
let mdl_options = [ key_to_space . mdl_key_value+ ]
let mod_options = [ key "options" . delimiter . mdl_options . eol ]


let option = simple_entry "raw"
           | simple_entry "net"
           | simple_entry "path"
           | simple_entry "disk_timeout"
           | simple_entry "core_collector"
           | simple_entry "link_delay"
           | simple_entry "kdump_post"
           | simple_entry "kdump_pre"
           | simple_entry "default"
           | fs
           | extra_bins
           | extra_modules
           | mod_options

(* View: lns
   The options lens
*)
let lns = ( empty | comment | option )*

let filter = incl "/etc/kdump.conf"

let xfm = transform lns filter
