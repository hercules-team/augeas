(*
Module: Solaris_ScsiVhci
  Parses /kernel/drv/scsi_vhci.conf on Solaris

Author: NuH <nuh.temp@gmail.com>

About: Reference
  This lens tries to keep as close as possible to file format.

About: Licence
  This file is licensed under the LGPL v2+, like the rest of Augeas.

About: Lens Usage

About: Configuration files
  This lens applies to /kernel/drv/scsi_vhci.conf on Solaris. See <filter>.
*)

module Solaris_ScsiVhci =
autoload xfm

(************************************************************************
 * Group:                 USEFUL PRIMITIVES
 ************************************************************************)

(* View: comment *)
let comment = IniFile.comment "#" "#"

(* View: key/value separator *)
let sep     = del /=/ "="

(* View: end of line including possible white spaces *)
let eol     = del /[ \t]*\n/ "\n"

(* View: hard end of line *)
let hardeol = del /\n/ "\n"

(* View: empty
    Map empty lines, including empty asterisk comments *)
let empty   = [ eol ]

(* View: quots chars *)
let quote   = del /\"/ "\""

(* View: key/value separator including possible white spaces *)
let equal = del /[ \t]*=/ " ="

(* View: key/value variables names
    like: load-balance="value"; *)
let entry_settings = "load-balance" | "auto-failback"

(************************************************************************
 * Group:                     COMMANDS
 ************************************************************************)

(* View: file_entry
    Function for line: name="scsi_vhci" class="root"; *)
let file_entry =
       let item = quote . store (/[^" \t\n]+/) . quote
    in let name = [ key "name" . sep .  item ]
    in let class = [ key "class" . sep . item ]
    in [ label "file" . Util.indent . name . Util.del_ws_spc . class . Util.del_str ";" . eol ]

(* View: entry
    Function for simple key/value setting commands such as load-balance *)
let entry = [ key entry_settings . sep . quote. store (/[^" \t\n]+/) . quote . Util.del_str ";" . eol]

(* View: inline_comment
    Function for comments inside the list of values *)
let inline_comment = [ label "#comment" . del /[ \n\t]*#[ \t]*/ "\n# " . store Rx.space_in . eol]

(* View: ddipath
    Function for list of 'ddi-forceload' values *)
let ddipath =
       let section = "path"
    in let item = quote . store /[^"\t\n]+/ . quote
    in let path = [seq section . inline_comment* . del /[ \t\n]*/ "\n\t" . item]
    in let comma = del /[ \n\t]*/ "" . del /,/ ","
    in let entry_list = path . (comma . path)*
    in let list = (counter section) . entry_list . Util.del_str ";"
    in Build.key_value_line_comment "ddi-forceload" equal list comment

(* View: ddipath
    Function for list of 'scsi-vhci-failover-override' values *)
let failoveroverride =
       let section = "drv"
    in let item = quote . store /[^"\t\n]+/ . quote
    in let inline_sep = del /[ \t]*,[ \t]*/ ", "
    in let entry = [seq section .
                        inline_comment* .
                        [ label "id" . del /[ \t\n]*/ "\n\t" . item ] .
                        inline_sep .
                        [ label "module" . item ] ]

    in let comma = del /[ \n\t]*/ "" . del /,/ ","
    in let entry_list = entry . (comma . entry)*
    in let list = (counter section) . entry_list . Util.del_str ";"
    in Build.key_value_line_comment "scsi-vhci-failover-override" equal list comment

(************************************************************************
 * Group:                     LENS
 ************************************************************************)

(* View: lns *)
let lns = ( comment | file_entry | entry | empty | ddipath | failoveroverride )*

(* Variable: filter *)
let filter = incl "/kernel/drv/scsi_vhci.conf"
    . Util.stdexcl

let xfm = transform lns filter
