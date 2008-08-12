(* Sudoers module for Augeas                  
 Author: Raphael Pinson <raphink@gmail.com> 
                                            
 Reference: `man sudoers`                   
 This lens tries to keep as close as possible to `man sudoers` where possible. 
 For example, recursive definitions such as:

     Cmnd_Spec_List ::= Cmnd_Spec |
                        Cmnd_Spec ',' Cmnd_Spec_List

 are replaced by 

     let cmnd_spec_list = cmnd_spec . ( sep_com . cmnd_spec )*     

 since Augeas cannot deal with recursive definitions.
 The definitions from `man sudoers` are put as commentaries for reference
 throughout the file. More information can be found in the manual.
*)



module Sudoers =
  autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol       = del /[ \t]*\n/ "\n"
let indent    = del /[ \t]+/ ""

(* Define separators *)
let sep_spc  = del /[ \t]+/ " " 
let sep_cont = del /([ \t]+|[ \t]*\\\\\n[ \t]*)/ " "
let sep_com  = sep_cont? . Util.del_str "," . sep_cont?
let sep_eq   = sep_cont? . Util.del_str "=" . sep_cont?
let sep_col  = sep_cont? . Util.del_str ":" . sep_cont?

(* Define fields *)
let sto_to_com_cmnd = store /([^,=:#() \t\n\\\\][^,=:#()\n\\\\]*[^,=:#() \t\n\\\\])|[^,=:#() \t\n\\\\]/
let sto_to_com      = store /[^,=:#() \t\n\\\\]+/

(* sto_to_com does not begin or end with a space *)
(* TODO: there could be a \ in the middle of a command... *)
let sto_to_com_user = store ( /[^,=:#() \t\n]+/ 
                              - /(User|Runas|Host|Cmnd)_Alias|Defaults.*/ )
let sto_to_eq  = store /[^,=:#() \t\n\\\\]+/
let sto_to_spc = store /[^() \t\n\\\\]+/


(* define comments and empty lines *)
let comment = 
  let value_to_eol = del /[ \t]*/ " " . store /([^ \t\n].*[^ \t\n]|[^ \t\n])/ in
  [ label "comment" . del /[ \t]*#/ "# " .  value_to_eol . eol ]
 
let empty   = [ del /[ \t]*#?[ \t]*\n/ "" ]

(************************************************************************
 *                                     ALIASES
 *************************************************************************)

(************************************************************************
 *   User_Alias ::= NAME '=' User_List
 *   Runas_Alias ::= NAME '=' Runas_List
 *   Host_Alias ::= NAME '=' Host_List
 *   Cmnd_Alias ::= NAME '=' Cmnd_List
 *************************************************************************)
let alias_field (kw:string) (sto:lens) = [ label kw . sto ]
let alias_list  (kw:string) (sto:lens) = 
  alias_field kw sto . ( sep_com . alias_field kw sto )*

(************************************************************************
 *  NAME ::= [A-Z]([A-Z][0-9]_)*
 *************************************************************************)
let alias_name
    = [ label "name" . store /[A-Z][A-Z0-9_]*/ ]

(************************************************************************
 *  Alias_Type NAME = item1, item2, ... 
 *************************************************************************)
let alias_entry_single (field:string) (sto:lens)
    = [ label "alias" . alias_name . sep_eq . alias_list field sto ]

(************************************************************************
 *  Alias_Type NAME = item1, item2, item3 : NAME = item4, item5
 *************************************************************************)
let alias_entry (kw:string) (field:string) (sto:lens)
    = [ indent? . key kw . sep_cont . alias_entry_single field sto
          . ( sep_col . alias_entry_single field sto )* . eol ]

(* TODO: go further in user definitions *)
let user_alias  = alias_entry "User_Alias" "user" sto_to_com
let runas_alias = alias_entry "Runas_Alias" "runas_user" sto_to_com
let host_alias  = alias_entry "Host_Alias" "host" sto_to_com
let cmnd_alias  = alias_entry "Cmnd_Alias" "command" sto_to_com_cmnd


(************************************************************************
 *  Alias ::= 'User_Alias'  User_Alias (':' User_Alias)* |
 *            'Runas_Alias' Runas_Alias (':' Runas_Alias)* |
 *            'Host_Alias'  Host_Alias (':' Host_Alias)* |
 *            'Cmnd_Alias'  Cmnd_Alias (':' Cmnd_Alias)*
 *************************************************************************)
let alias = user_alias | runas_alias | host_alias | cmnd_alias



(************************************************************************
 *                                 DEFAULTS
 *************************************************************************)

(************************************************************************
 *  Default_Type ::= 'Defaults' |
 *                   'Defaults' '@' Host_List |
 *                   'Defaults' ':' User_List |
 *                   'Defaults' '>' Runas_List
 *************************************************************************)
let default_type     = 
  let value = store /[@:>][^ \t\n\\\\]+/ in
  [ label "type" . value ]

(************************************************************************
 *  Parameter ::= Parameter '=' Value |
 *                Parameter '+=' Value |
 *                Parameter '-=' Value |
 *                '!'* Parameter
 *************************************************************************)
let parameter        = 
  let value = /([^,:= \t\n\\\\][^,\n\\\\]*[^, \t\n\\\\])|[^,:= \t\n\\\\]/ in
  [ label "parameter" . store value ]

(************************************************************************
 *  Parameter_List ::= Parameter |
 *                     Parameter ',' Parameter_List
 *************************************************************************)
let parameter_list   = parameter . ( sep_com . parameter )*
    
(************************************************************************
 *  Default_Entry ::= Default_Type Parameter_List
 *************************************************************************)
let defaults = [ indent? . key "Defaults" . default_type? . sep_cont 
                   . parameter_list . eol ]



(************************************************************************
 *                              USER SPECIFICATION 
 *************************************************************************)

(************************************************************************
 *  Runas_Spec ::= '(' Runas_List ')'
 *************************************************************************)
let runas_spec = Util.del_str "(" . alias_list "runas_user" sto_to_com 
    . Util.del_str ")" . sep_cont?

(************************************************************************
 * Tag_Spec ::= ('NOPASSWD:' | 'PASSWD:' | 'NOEXEC:' | 'EXEC:' |
 *              'SETENV:' | 'NOSETENV:')
 *************************************************************************)
let tag_spec   =
  [ label "tag" . store /(NO)?(PASSWD|EXEC|SETENV)/ . sep_col ]

(************************************************************************
 *  Cmnd_Spec ::= Runas_Spec? Tag_Spec* Cmnd
 *************************************************************************)
let cmnd_spec  =
  [ label "command" .  runas_spec? . tag_spec* . sto_to_com_cmnd ]

(************************************************************************
 * Cmnd_Spec_List ::= Cmnd_Spec |
 *                    Cmnd_Spec ',' Cmnd_Spec_List
 *************************************************************************)
let cmnd_spec_list = cmnd_spec . ( sep_com . cmnd_spec )*


(************************************************************************
 * User_Spec ::= User_List Host_List '=' Cmnd_Spec_List \
 *               (':' Host_List '=' Cmnd_Spec_List)*
 *************************************************************************)
let spec_list = [ label "host_group" . alias_list "host" sto_to_com 
                    . sep_eq . cmnd_spec_list ]

let spec = [ label "spec" . indent?
               . alias_list "user" sto_to_com_user . sep_cont
               . spec_list
    	       . ( sep_col . spec_list )* . eol ]


(************************************************************************
 *                              LENS & FILTER
 *************************************************************************)

let lns = ( empty | comment | alias | defaults | spec  )*

let filter = (incl "/etc/sudoers")
    . Util.stdexcl

let xfm = transform lns filter
