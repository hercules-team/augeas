(* IniFile generic module for Augeas          *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)
(* TODO: Support double quotes in value       *)

module IniFile  =

    (* Define useful shortcuts *)

    let eol                = Util.del_str "\n"
    let del_to_eol         = del /[^\n]*/ ""
    let value_sep          = del /[ \t]*=[ \t]*/ " = "
    let value_sepwithcolon = del /[ \t]*(=|:)[ \t]*/ " = "
    let value_to_eol       = store /([^ \t\n][^\n]*)?/


    (* Define entry function *)
    (* Some implementations of INI file allow ";" as separator *)
    let entry (kw:regexp) = [ key kw . value_sepwithcolon . value_to_eol . eol ]
    let entry_nocolon (kw:regexp) = [ key kw . value_sep . value_to_eol . eol ]

    (* Define comment and empty strings *)
    (* Some implementations of INI file allow "#" as a comment sign *)
    let comment = [ label "comment" . del /(#|;)[ \t]*/ "; " .  store /([^ \t\n][^\n]*)?/ . eol ]
    let comment_nosharp = [ label "comment" . del /;[ \t]*/ "; " .  store /([^ \t\n][^\n]*)?/ . eol ]

    let empty  = [ del /[ \t]*/ "" . eol ]


    (* Define record *)

    let title = Util.del_str "[" . store /[^] ]+/ . Util.del_str "]". eol
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
