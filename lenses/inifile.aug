(* IniFile generic module for Augeas          *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)
(* TODO: Support double quotes in value       *)

module IniFile  =


(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

(* Internal primitives *)
let eol                = Util.eol
let empty              = [ eol ]


(* Define sep and defaults *)
let sep (pat:regexp) (default:string) 
                       = Util.del_opt_ws "" . del pat default
let sep_re             = /[=:]/
let sep_default        = "="


(* Define sto_*  *)
let sto_to_eol         = Util.del_opt_ws ""
                         . store /([^ \t\n].*[^ \t\n]|[^ \t\n])/
let sto_to_comment     = Util.del_opt_ws ""
                         . store /[^;# \t\n][^;#\n]*[^;# \t\n]|[^;# \t\n]/


(* Define comment and defaults *)
let comment (pat:regexp) (default:string)
                       = [ label "#comment" . sep pat default
		         . sto_to_eol? . eol ]
let comment_re         = /[;#]/
let comment_default    = ";"


(************************************************************************
 *                             ENTRY
 *************************************************************************)

(* entry includes comments *)

let entry (kw:regexp) (sep:lens) (comment:lens)
                       = [ key kw . sep . sto_to_comment? . (comment|eol) ] | comment
let entry_re           = ( /[A-Za-z][A-Za-z0-9\._-]+/ - /#comment/ )


(************************************************************************
 *                             RECORD 
 *************************************************************************)

let title (kw:regexp)
                       = Util.del_str "[" . key kw
                         . Util.del_str "]". eol
let title_label (name:string) (kw:regexp)
                       = label name
                         . Util.del_str "[" . store kw
                         . Util.del_str "]". eol

let record_noempty (title:lens) (entry:lens)
                       = [ title
		       . entry* ]
let record (title:lens) (entry:lens)
                       = record_noempty title ( entry | empty )
let record_re          = ( /[^]\n\/]+/ - /#comment/ ) 
let record_label_re    = /[^]\n]+/


(************************************************************************
 *                              LENS
 *************************************************************************)

let lns_noempty (record:lens) (comment:lens)
                       = comment* . record*
let lns (record:lens) (comment:lens)
                       = lns_noempty record (comment|empty)


