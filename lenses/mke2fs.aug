(*
Module: Mke2fs
  Parses /etc/mke2fs.conf

Author: Raphael Pinson <raphink@gmail.com>

About: Reference
  This lens tries to keep as close as possible to `man 5 mke2fs.conf` where possible.

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/mke2fs.conf. See <filter>.
*)


module Mke2fs =
  autoload xfm

(************************************************************************
 * Group:                 USEFUL PRIMITIVES
 *************************************************************************)

(* View: comment *)
let comment = IniFile.comment IniFile.comment_re IniFile.comment_default

(* View: sep *)
let sep = IniFile.sep /=[ \t]*/ "="

(* View: empty *)
let empty = IniFile.empty


(************************************************************************
 * Group:                 RECORD TYPES
 *************************************************************************)


(* View: entry
    A generic entry for lens lns *)
let entry (kw:regexp) (lns:lens) = Build.key_value_line kw sep lns


(* View: list_sto
    A list of values with given lens *)
let list_sto (kw:regexp) (lns:lens) = counter "item" .
                                          entry kw
                                            (Build.opt_list 
                                              [lns]
                                              Sep.comma)

(* View: entry_sto
    Store a regexp as entry value *)
let entry_sto (kw:regexp) (val:regexp) = entry kw (store val)


(************************************************************************
 * Group:                 COMMON ENTRIES
 *************************************************************************)
(* View: common_entry
     Entries shared between <defaults> and <fs_types> sections *)
let common_entry   = list_sto ("base_features"|"default_features")
                        (key Rx.word)
                   | entry_sto "blocksize" ("-"? . Rx.integer)
                   | entry_sto "hash_alg" ("legacy"|"half_md4"|"tea")
                   | entry_sto ("inode_ratio"|"inode_size") Rx.integer

(************************************************************************
 * Group:                 DEFAULTS SECTION
 *************************************************************************)

(* View: defaults_entry
    Possible entries under the <defaults> section *)
let defaults_entry = entry_sto "force_undo" ("true"|"false")
                   | entry_sto "fs_type" Rx.word
                   | entry_sto "undo_dir" Rx.fspath
                   | list_sto "default_mntopts" (key Rx.word)
                   | entry_sto "enable_periodic_fsck" Rx.integer
                   
(* View: defaults_title
    Title for the <defaults> section *)
let defaults_title  = IniFile.title "defaults"

(* View: defaults
    A defaults section *)
let defaults = IniFile.record defaults_title
                  ((Util.indent . (defaults_entry|common_entry)) | comment)


(************************************************************************
 * Group:                 FS_TYPES SECTION
 *************************************************************************)

(* View: fs_types_entry
    Possible entries under a <fs_types_record> group *)
let fs_types_entry =list_sto "features"
                        ([del /\^/ "^" . label "disable"]?
                                           . key Rx.word)
                   | list_sto "options"
                        (key Rx.word . Util.del_str "="
                       . store Rx.word)
                   | entry_sto "lazy_itable_init" ("true"|"false")
                   | entry_sto ("flex_bg_size"|"auto_64-bit_support")
                       Rx.integer

(* View: fs_types_record
     Fs group records under the <fs_types> section *)
let fs_types_record = [ label "filesystem"
                     . Util.indent . store Rx.word
                     . del /[ \t]*=[ \t]*\{[ \t]*\n/ " = {\n"
                     . ((Util.indent . (fs_types_entry|common_entry)) | empty | comment)*
                     . del /[ \t]*\}[ \t]*\n/ " }\n" ]

(* View: fs_types_title
    Title for the <fs_types> section *)
let fs_types_title = IniFile.title "fs_types"

(* View: fs_types
    A fs_types section *)
let fs_types = IniFile.record fs_types_title
                  (fs_types_record | comment)


(************************************************************************
 * Group:                 LENS AND FILTER
 *************************************************************************)

(* View: lns
     The mke2fs lens
*)
let lns = (empty|comment)* . (defaults|fs_types)*

(* Variable: filter *)
let filter = incl "/etc/mke2fs.conf"

let xfm = transform lns filter


