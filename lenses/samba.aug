(* Samba module for Augeas 
   Author: Free Ekanayaka <free@64studio.com>

   Reference: man smb.conf(5)

*)


module Samba =
  autoload xfm

(************************************************************************
 * INI File settings
 *************************************************************************)

let comment  = IniFile.comment IniFile.comment_re IniFile.comment_default
let sep      = IniFile.sep IniFile.sep_re IniFile.sep_default


(* Import useful INI File primitives *)
let eol      = IniFile.eol
let empty    = IniFile.empty
let sto_to_comment
             = IniFile.sto_to_comment

(************************************************************************
 *                        ENTRY
 * smb.conf allows indented entries
 *************************************************************************)

let entry_re = /[A-Za-z0-9_.-][A-Za-z0-9 _.-]*[A-Za-z0-9_.-]/
let entry    = let kw = entry_re in
             [ Util.indent
             . key kw
             . sep
             . sto_to_comment?
             . (comment|eol) ]
             | comment

(************************************************************************
 *                         TITLE
 *************************************************************************)

let title    = IniFile.title_label "target" IniFile.record_label_re
let record   = IniFile.record title entry 

(************************************************************************
 *                         LENS & FILTER
 *************************************************************************)

let lns      = IniFile.lns record comment

let filter   = (incl "/etc/samba/smb.conf")
             . Util.stdexcl

let xfm = transform lns filter
