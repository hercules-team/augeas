(* IniFile generic module for Augeas          *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)
(* TODO: Support double quotes in value       *)

module IniFile  =

    (* Define useful shortcuts *)

    let eol                = del /[ \t]*\n/ "\n"
    let value_sep          = del /[ \t]*=/ " = "
    let value_sepwithcolon = del /[ \t]*(=|:)/ " = "
    let value_to_eol       = del /[ \t]*/ " " . store /([^ \t\n].*[^ \t\n]|[^ \t\n])/
    let value_to_comment   = del /[ \t]*/ "" . store /[^;# \t\n]+/


    (* Define comment and empty strings *)
    (* Some implementations of INI file allow "#" as a comment sign *)
    let comment_generic (pattern:regexp) = [ label "comment" . del pattern "; " .  value_to_eol? . eol ]
    let comment = comment_generic /[ \t]*(#|;)/
    let comment_nosharp = comment_generic /[ \t]*;/

    let empty  = [ del /[ \t]*\n/ "" ]


    (* Define entry function *)
    (* Some implementations of INI file allow ";" as separator *)
    let entry_generic (kw:regexp) (sep:lens) (comment:lens) = [ key kw . sep . value_to_comment? . (comment|eol) ]
    let entry (kw:regexp)                                   = entry_generic kw value_sepwithcolon comment
    let entry_setcomment (kw:regexp) (comment:lens)         = entry_generic kw value_sepwithcolon comment
    let entry_nocolon (kw:regexp)                           = entry_generic kw value_sep comment
    let entry_nocolon_setcomment (kw:regexp) (comment:lens) = entry_generic kw value_sep comment


    (* Define record *)

    let title = Util.del_str "[" . store /[^]]+/ . Util.del_str "]". eol
    let record (label_name:string) (entry:lens) = [ label label_name . title . (entry | comment | empty)* ] 
    let record_setcomment (label_name:string) (entry:lens) (comment:lens) = [ label label_name . title . (entry | comment | empty)* ] 
    (* Some implementations of INI File do not allow empty lines *)
    let record_noempty (label_name:string) (entry:lens) = [ label label_name . title . (entry | comment)* ] 
    let record_noempty_setcomment (label_name:string) (entry:lens) (comment:lens) = [ label label_name . title . (entry | comment)* ] 

    (* Generic INI file lens *)
    let lns (record:lens) = ( comment | empty )* . record*
    (* Let the user choose the type of comment they want *)
    let lns_setcomment (record:lens) (comment:lens) = ( comment | empty )* . record*

    (* Some implementations of INI File do not allow empty lines *)
    let lns_noempty (record:lens) = comment* . record*
    (* Let the user choose the type of comment they want *)
    let lns_noempty_setcomment (record:lens) (comment:lens) = comment* . record*
