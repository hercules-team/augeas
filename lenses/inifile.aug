(*
Module: IniFile
  Generic module to create INI files lenses

Author: Raphael Pinson <raphink@gmail.com>

About: License
  This file is licensed under the LGPL v2+, like the rest of Augeas.

About: TODO
  Things to add in the future
  - Support double quotes in value

About: Lens usage
  This lens is made to provide generic primitives to construct INI File lenses.
  See <Puppet>, <PHP>, <MySQL> or <Dput> for examples of real life lenses using it.

About: Examples
  The <Test_IniFile> file contains various examples and tests.
*)

module IniFile  =


(************************************************************************
 * Group:               USEFUL PRIMITIVES
 *************************************************************************)

(* Group: Internal primitives *)

(*
Variable: eol
  End of line, inherited from <Util.eol>
*)
let eol                = Util.eol

(*
View: empty
  Empty line, an <eol> subnode
*)
let empty              = [ eol ]


(* Group: Separators *)



(*
Variable: sep
  Generic separator

  Parameters:
    pat:regexp - the pattern to delete
    default:string - the default string to use
*)
let sep (pat:regexp) (default:string)
                       = Sep.opt_space . del pat default

(*
Variable: sep_noindent
  Generic separator, no indentation

  Parameters:
    pat:regexp - the pattern to delete
    default:string - the default string to use
*)
let sep_noindent (pat:regexp) (default:string)
                       = del pat default

(*
Variable: sep_re
  The default regexp for a separator
*)

let sep_re             = /[=:]/

(*
Variable: sep_default
  The default separator value
*)
let sep_default        = "="


(* Group: Stores *)


(*
Variable: sto_to_eol
  Store until end of line
*)
let sto_to_eol         = Sep.opt_space . store Rx.space_in

(*
Variable: to_comment_re
  Regex until comment
*)
let to_comment_re = /[^";# \t\n][^";#\n]*[^";# \t\n]|[^";# \t\n]/
                  | /"[^\n"]*"/

(*
Variable: sto_to_comment
  Store until comment
*)
let sto_to_comment = Sep.opt_space . store to_comment_re

(*
Variable: sto_multiline
  Store multiline values
*)
let sto_multiline = Sep.opt_space
         . store (to_comment_re
               . (/[ \t]*\n/ . Rx.space . to_comment_re)*)

(*
Variable: sto_multiline_nocomment
  Store multiline values without an end-of-line comment
*)
let sto_multiline_nocomment = Sep.opt_space
         . store (Rx.space_in . (/[ \t]*\n/ . Rx.space . Rx.space_in)*)


(* Group: Define comment and defaults *)

(*
View: comment
  Map comments into "#comment" nodes

  Parameters:
    pat:regexp - pattern to delete before commented data
    default:string - default pattern before commented data

  Sample Usage:
  (start code)
    let comment  = IniFile.comment "#" "#"
    let comment  = IniFile.comment IniFile.comment_re IniFile.comment_default
  (end code)
*)
let comment (pat:regexp) (default:string)
                       = [ label "#comment" . sep pat default
		         . sto_to_eol? . eol ]

let comment_noindent (pat:regexp) (default:string)
                       = [ label "#comment" . sep_noindent pat default
		         . sto_to_eol? . eol ]
(*
Variable: comment_re
  Default regexp for <comment> pattern
*)

let comment_re         = /[;#]/

(*
Variable: comment_default
  Default value for <comment> pattern
*)
let comment_default    = ";"


(************************************************************************
 * Group:                     ENTRY
 *************************************************************************)

(* Group: entry includes comments *)

(*
View: entry
  Generic INI File entry

  Parameters:
    kw:regexp    - keyword regexp for the label
    sep:lens     - lens to use as key/value separator
    comment:lens - lens to use as comment

  Sample Usage:
     > let entry = IniFile.entry setting sep comment
*)
let entry (kw:regexp) (sep:lens) (comment:lens)
                       = [ key kw . sep . sto_to_comment? . (comment|eol) ] | comment


(*
View: entry_multiline
  Generic multiline INI File entry

  Parameters:
    kw:regexp    - keyword regexp for the label
    sep:lens     - lens to use as key/value separator
    comment:lens - lens to use as comment
*)
let entry_multiline (kw:regexp) (sep:lens) (comment:lens)
                       = [ key kw . sep . sto_multiline? . (comment|eol) ]
                         | comment

(*
View: entry_multiline_nocomment
  Generic multiline INI File entry without an end-of-line comment

  Parameters:
    kw:regexp    - keyword regexp for the label
    sep:lens     - lens to use as key/value separator
    comment:lens - lens to use as comment
*)
let entry_multiline_nocomment (kw:regexp) (sep:lens) (comment:lens)
                       = [ key kw . sep . sto_multiline_nocomment? . eol ]
                         | comment


(*
View: indented_entry
  Generic INI File entry that might be indented with an arbitrary
  amount of whitespace

  Parameters:
    kw:regexp    - keyword regexp for the label
    sep:lens     - lens to use as key/value separator
    comment:lens - lens to use as comment

  Sample Usage:
     > let entry = IniFile.indented_entry setting sep comment
*)
let indented_entry (kw:regexp) (sep:lens) (comment:lens)
                       = [ Util.del_opt_ws "" .
                           key kw . sep . sto_to_comment? .
                           (comment|eol)
                         ]
                         | comment
(*
View: entry_list
  Generic INI File list entry

  Parameters:
    kw:regexp     - keyword regexp for the label
    sep:lens      - lens to use as key/value separator
    sto:regexp    - store regexp for the values
    list_sep:lens - lens to use as list separator
    comment:lens  - lens to use as comment
*)
let entry_list (kw:regexp) (sep:lens) (sto:regexp) (list_sep:lens) (comment:lens) =
  let list = counter "elem"
      . Build.opt_list [ seq "elem" . store sto ] list_sep
  in Build.key_value_line_comment kw sep (Sep.opt_space . list) comment

(*
View: entry_list_nocomment
  Generic INI File list entry without an end-of-line comment

  Parameters:
    kw:regexp     - keyword regexp for the label
    sep:lens      - lens to use as key/value separator
    sto:regexp    - store regexp for the values
    list_sep:lens - lens to use as list separator
*)
let entry_list_nocomment (kw:regexp) (sep:lens) (sto:regexp) (list_sep:lens) =
  let list = counter "elem"
      . Build.opt_list [ seq "elem" . store sto ] list_sep
  in Build.key_value_line kw sep (Sep.opt_space . list)

(*
Variable: entry_re
  Default regexp for <entry> keyword
*)
let entry_re           = ( /[A-Za-z][A-Za-z0-9._-]+/ )


(************************************************************************
 * Group:                      RECORD
 *************************************************************************)

(* Group: Title definition *)

(*
View: title
  Title for <record>. This maps the title of a record as a node in the abstract tree.

  Parameters:
    kw:regexp - keyword regexp for the label

  Sample Usage:
    > let title   = IniFile.title IniFile.record_re
*)
let title (kw:regexp)
                       = Util.del_str "[" . key kw
                         . Util.del_str "]". eol

(*
View: indented_title
  Title for <record>. This maps the title of a record as a node in the abstract tree. The title may be indented with arbitrary amounts of whitespace

  Parameters:
    kw:regexp - keyword regexp for the label

  Sample Usage:
    > let title   = IniFile.title IniFile.record_re
*)
let indented_title (kw:regexp)
                       = Util.indent . title kw

(*
View: title_label
  Title for <record>. This maps the title of a record as a value in the abstract tree.

  Parameters:
    name:string - name for the title label
    kw:regexp   - keyword regexp for the label

  Sample Usage:
    > let title   = IniFile.title_label "target" IniFile.record_label_re
*)
let title_label (name:string) (kw:regexp)
                       = label name
                         . Util.del_str "[" . store kw
                         . Util.del_str "]". eol

(*
View: indented_title_label
  Title for <record>. This maps the title of a record as a value in the abstract tree. The title may be indented with arbitrary amounts of whitespace

  Parameters:
    name:string - name for the title label
    kw:regexp   - keyword regexp for the label

  Sample Usage:
    > let title   = IniFile.title_label "target" IniFile.record_label_re
*)
let indented_title_label (name:string) (kw:regexp)
                       = Util.indent . title_label name kw


(*
Variable: record_re
  Default regexp for <title> keyword pattern
*)
let record_re          = ( /[^]\n\/]+/ - /#comment/ )

(*
Variable: record_label_re
  Default regexp for <title_label> keyword pattern
*)
let record_label_re    = /[^]\n]+/


(* Group: Record definition *)

(*
View: record_noempty
  INI File Record with no empty lines allowed.

  Parameters:
    title:lens - lens to use for title. Use either <title> or <title_label>.
    entry:lens - lens to use for entries in the record. See <entry>.
*)
let record_noempty (title:lens) (entry:lens)
                       = [ title
		       . entry* ]

(*
View: record
  Generic INI File record

  Parameters:
    title:lens - lens to use for title. Use either <title> or <title_label>.
    entry:lens - lens to use for entries in the record. See <entry>.

  Sample Usage:
    > let record  = IniFile.record title entry
*)
let record (title:lens) (entry:lens)
                       = record_noempty title ( entry | empty )


(************************************************************************
 * Group:                      LENS
 *************************************************************************)


(*

Group: Lens definition

View: lns_noempty
  Generic INI File lens with no empty lines

  Parameters:
    record:lens  - record lens to use. See <record_noempty>.
    comment:lens - comment lens to use. See <comment>.

  Sample Usage:
    > let lns     = IniFile.lns_noempty record comment
*)
let lns_noempty (record:lens) (comment:lens)
                       = comment* . record*

(*
View: lns
  Generic INI File lens

  Parameters:
    record:lens  - record lens to use. See <record>.
    comment:lens - comment lens to use. See <comment>.

  Sample Usage:
    > let lns     = IniFile.lns record comment
*)
let lns (record:lens) (comment:lens)
                       = lns_noempty record (comment|empty)


