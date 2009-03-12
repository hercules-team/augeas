(* 
Module: Sudoers
  Parses /etc/sudoers     

Author: Raphael Pinson <raphink@gmail.com> 
                                            
About: Reference
  This lens tries to keep as close as possible to `man sudoers` where possible.

For example, recursive definitions such as

     > Cmnd_Spec_List ::= Cmnd_Spec |
     >                    Cmnd_Spec ',' Cmnd_Spec_List

are replaced by

  >   let cmnd_spec_list = cmnd_spec . ( sep_com . cmnd_spec )*

since Augeas cannot deal with recursive definitions.
The definitions from `man sudoers` are put as commentaries for reference
throughout the file. More information can be found in the manual.

About: License
  This file is licensed under the LGPLv2+, like the rest of Augeas.


About: Lens Usage
  Sample usage of this lens in augtool

    * Set first Defaults to apply to the "LOCALNET" network alias
      > set /files/etc/sudoers/Defaults[1]/type "@LOCALNET"
    * List all user specifications applying explicitely to the "admin" Unix group
      > match /files/etc/sudoers/spec/user "%admin"
    * Remove the full 3rd user specification
      > rm /files/etc/sudoers/spec[3]

About: Configuration files
  This lens applies to /etc/sudoers. See <filter>.
*)



module Sudoers =
  autoload xfm

(************************************************************************
 * Group:                 USEFUL PRIMITIVES
 *************************************************************************)

(* Group: Generic primitives *)
(* Variable: eol *)
let eol       = del /[ \t]*\n/ "\n"

(* Variable: indent *)
let indent    = del /[ \t]*/ ""


(* Group: Separators *)

(* Variable: sep_spc *)
let sep_spc  = del /[ \t]+/ " "

(* Variable: sep_cont *)
let sep_cont = del /([ \t]+|[ \t]*\\\\\n[ \t]*)/ " "

(* Variable: sep_conf_opt *)
let sep_cont_opt = del /([ \t]*|[ \t]*\\\\\n[ \t]*)/ " "

(* Variable: sep_com *)
let sep_com  = sep_cont_opt . Util.del_str "," . sep_cont_opt

(* Variable: sep_eq *)
let sep_eq   = sep_cont_opt . Util.del_str "=" . sep_cont_opt

(* Variable: sep_col *)
let sep_col  = sep_cont_opt . Util.del_str ":" . sep_cont_opt


(* Group: Stores *)

(* Variable: sto_to_com_cmnd
sto_to_com_cmnd does not begin or end with a space

TODO: there could be a \ in the middle of a command... *)
let sto_to_com_cmnd = store /([^,=:#() \t\n\\\\][^,=:#()\n\\\\]*[^,=:#() \t\n\\\\])|[^,=:#() \t\n\\\\]/

(* Variable: sto_to_com *)
let sto_to_com      = store /[^,=:#() \t\n\\\\]+/

(* Variable: sto_to_com_user *)
let sto_to_com_user = store ( /[^,=:#() \t\n]+/
                              - /(User|Runas|Host|Cmnd)_Alias|Defaults.*/ )

(* Variable: sto_to_eq *)
let sto_to_eq  = store /[^,=:#() \t\n\\\\]+/

(* Variable: sto_to_spc *)
let sto_to_spc = store /[^() \t\n\\\\]+/


(* Group: Comments and empty lines *)

(* View: comment
Map comments in "#comment" nodes *)
let comment =
  let sto_to_eol = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/ in
  [ label "#comment" . del /[ \t]*#[ \t]*/ "# " . sto_to_eol . eol ]

(* View: empty
Map empty lines *)
let empty   = [ del /[ \t]*#?[ \t]*\n/ "\n" ]

(************************************************************************
 * Group:                                    ALIASES
 *************************************************************************)

(************************************************************************
 * View: alias_field
 *   Generic alias field to gather all Alias definitions
 *
 *   Definition:
 *     > User_Alias ::= NAME '=' User_List
 *     > Runas_Alias ::= NAME '=' Runas_List
 *     > Host_Alias ::= NAME '=' Host_List
 *     > Cmnd_Alias ::= NAME '=' Cmnd_List
 *
 *   Parameters:
 *     kw:string - the label string
 *     sto:lens  - the store lens
 *************************************************************************)
let alias_field (kw:string) (sto:lens) = [ label kw . sto ]

(* View: alias_list
     List of <alias_fields>, separated by commas *)
let alias_list  (kw:string) (sto:lens) =
  alias_field kw sto . ( sep_com . alias_field kw sto )*

(************************************************************************
 * View: alias_name
 *   Name of an <alias_entry_single>
 *
 *   Definition:
 *     > NAME ::= [A-Z]([A-Z][0-9]_)*
 *************************************************************************)
let alias_name
    = [ label "name" . store /[A-Z][A-Z0-9_]*/ ]

(************************************************************************
 * View: alias_entry_single
 *   Single <alias_entry>, named using <alias_name> and listing <alias_list>
 *
 *   Definition:
 *     > Alias_Type NAME = item1, item2, ...
 *
 *   Parameters:
 *     field:string - the field name, passed to <alias_list>
 *     sto:lens     - the store lens, passed to <alias_list>
 *************************************************************************)
let alias_entry_single (field:string) (sto:lens)
    = [ label "alias" . alias_name . sep_eq . alias_list field sto ]

(************************************************************************
 * View: alias_entry
 *   Alias entry, a list of comma-separated <alias_entry_single> fields
 *
 *   Definition:
 *     > Alias_Type NAME = item1, item2, item3 : NAME = item4, item5
 *
 *   Parameters:
 *     kw:string    - the alias keyword string
 *     field:string - the field name, passed to <alias_entry_single>
 *     sto:lens     - the store lens, passed to <alias_entry_single>
 *************************************************************************)
let alias_entry (kw:string) (field:string) (sto:lens)
    = [ indent . key kw . sep_cont . alias_entry_single field sto
          . ( sep_col . alias_entry_single field sto )* . eol ]

(* TODO: go further in user definitions *)
(* View: user_alias
     User_Alias, see <alias_field> *)
let user_alias  = alias_entry "User_Alias" "user" sto_to_com
(* View: runas_alias
     Run_Alias, see <alias_field> *)
let runas_alias = alias_entry "Runas_Alias" "runas_user" sto_to_com
(* View: host_alias
     Host_Alias, see <alias_field> *)
let host_alias  = alias_entry "Host_Alias" "host" sto_to_com
(* View: cmnd_alias
     Cmnd_Alias, see <alias_field> *)
let cmnd_alias  = alias_entry "Cmnd_Alias" "command" sto_to_com_cmnd


(************************************************************************
 * View: alias
 *   Every kind of Alias entry,
 *     see <user_alias>, <runas_alias>, <host_alias> and <cmnd_alias>
 *
 *   Definition:
 *     > Alias ::= 'User_Alias'  User_Alias (':' User_Alias)* |
 *     >           'Runas_Alias' Runas_Alias (':' Runas_Alias)* |
 *     >           'Host_Alias'  Host_Alias (':' Host_Alias)* |
 *     >           'Cmnd_Alias'  Cmnd_Alias (':' Cmnd_Alias)*
 *************************************************************************)
let alias = user_alias | runas_alias | host_alias | cmnd_alias
(************************************************************************
 * Group:                          DEFAULTS
 *************************************************************************)

(************************************************************************
 * View: default_type
 *   Type definition for <defaults>
 *
 *   Definition:
 *     > Default_Type ::= 'Defaults' |
 *     >                  'Defaults' '@' Host_List |
 *     >                  'Defaults' ':' User_List |
 *     >                  'Defaults' '>' Runas_List
 *************************************************************************)
let default_type     =
  let value = store /[@:>][^ \t\n\\\\]+/ in
  [ label "type" . value ]

(************************************************************************
 * View: parameter
 *   A single parameter for <defaults>
 *
 *   Definition:
 *     > Parameter ::= Parameter '=' Value |
 *     >               Parameter '+=' Value |
 *     >               Parameter '-=' Value |
 *     >               '!'* Parameter
 *************************************************************************)
let parameter        =
  let value = /([^,:= \t\n\\\\][^,\n\\\\]*[^, \t\n\\\\])|[^,:= \t\n\\\\]/ in
  [ label "parameter" . store value ]

(************************************************************************
 * View: paramater_list
 *   A list of comma-separated <parameters> for <defaults>
 *
 *   Definition:
 *     > Parameter_List ::= Parameter |
 *     >                    Parameter ',' Parameter_List
 *************************************************************************)
let parameter_list   = parameter . ( sep_com . parameter )*

(************************************************************************
 * View: defaults
 *   A Defaults entry
 *
 *   Definition:
 *     > Default_Entry ::= Default_Type Parameter_List
 *************************************************************************)
let defaults = [ indent . key "Defaults" . default_type? . sep_cont
                   . parameter_list . eol ]



(************************************************************************
 * Group:                     USER SPECIFICATION
 *************************************************************************)

(************************************************************************
 * View: runas_spec
 *   A runas specification for <spec>, using <alias_list>
 *
 *   Definition:
 *     > Runas_Spec ::= '(' Runas_List ')'
 *************************************************************************)
let runas_spec = Util.del_str "(" . alias_list "runas_user" sto_to_com
    . Util.del_str ")" . sep_cont_opt

(************************************************************************
 * View: tag_spec
 *   Tag specification for <spec>
 *
 *   Definition:
 *     > Tag_Spec ::= ('NOPASSWD:' | 'PASSWD:' | 'NOEXEC:' | 'EXEC:' |
 *     >              'SETENV:' | 'NOSETENV:')
 *************************************************************************)
let tag_spec   =
  [ label "tag" . store /(NO)?(PASSWD|EXEC|SETENV)/ . sep_col ]

(************************************************************************
 * View: cmnd_spec
 *   Command specification for <spec>,
 *     with optional <runas_spec> and any amount of <tag_specs>
 *
 *   Definition:
 *     > Cmnd_Spec ::= Runas_Spec? Tag_Spec* Cmnd
 *************************************************************************)
let cmnd_spec  =
  [ label "command" .  runas_spec? . tag_spec* . sto_to_com_cmnd ]

(************************************************************************
 * View: cmnd_spec_list
 *   A list of comma-separated <cmnd_specs>
 *
 *   Definition:
 *     > Cmnd_Spec_List ::= Cmnd_Spec |
 *     >                    Cmnd_Spec ',' Cmnd_Spec_List
 *************************************************************************)
let cmnd_spec_list = cmnd_spec . ( sep_com . cmnd_spec )*


(************************************************************************
 * View: spec_list
 *   Group of hosts with <cmnd_spec_list>
 *************************************************************************)
let spec_list = [ label "host_group" . alias_list "host" sto_to_com
                    . sep_eq . cmnd_spec_list ]

(************************************************************************
 * View: spec
 *   A user specification, listing colon-separated <spec_lists>
 *
 *   Definition:
 *     > User_Spec ::= User_List Host_List '=' Cmnd_Spec_List \
 *     >               (':' Host_List '=' Cmnd_Spec_List)*
 *************************************************************************)
let spec = [ label "spec" . indent
               . alias_list "user" sto_to_com_user . sep_cont
               . spec_list
               . ( sep_col . spec_list )* . eol ]


(************************************************************************
 * Group:                        LENS & FILTER
 *************************************************************************)

(* View: lns
     The sudoers lens, any amount of
       * <empty> lines
       * <comments>
       * <aliases>
       * <defaults>
       * <specs>
*)
let lns = ( empty | comment | alias | defaults | spec  )*

(* Variable: filter *)
let filter = (incl "/etc/sudoers")
    . Util.stdexcl

let xfm = transform lns filter


