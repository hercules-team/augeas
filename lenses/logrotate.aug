(* Logrotate module for Augeas                *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)
(* Supported :                                *)
(*   - defaults                               *)
(*   - rules                                  *)
(*   - (pre|post)rotate entries               *)
(*                                            *)
(* Todo :                                     *)
(*                                            *)

module Logrotate = 
   autoload xfm

   let sep_spc = Util.del_ws_spc
   let eol = Util.del_str "\n"
   let num = /[0-9]+/
   let word = /[^,# \n\t{}]+/
   

   (* define comments and empty lines *)
   let comment (indent:string) = [ label "comment" . del /[ \t]*/ indent . del /#[ \t]*/ "# " .  store /([^ \t\n][^\n]*)?/ . eol ]
   let empty   = [ del /[ \t]*\n/ "\n" ]


   (* Useful functions *)

   let select_to_eol (kw:string) (select:regexp) (indent:string) = [ del /[ \t]*/ indent . label kw . store select . eol ]
   let value_to_eol (kw:string) (value:regexp) (indent:string )  = [ del /[ \t]*/ indent . key kw . sep_spc . store value . eol ]
   let flag_to_eol (kw:string) (indent:string)                   = [ del /[ \t]*/ indent . key kw . eol ]


   (* Defaults *)

   let create (indent:string ) = [ del /[ \t]*/ indent . key "create" .
                       ( sep_spc . [ label "mode" . store num ] . sep_spc .
		       [ label "owner" . store word ] . sep_spc .
		       [ label "group" . store word ])?
		    . eol ]

   let attrs (indent:string) = select_to_eol "schedule" /(daily|weekly|monthly)/ indent
                | value_to_eol "rotate" num indent
		| create indent
		| flag_to_eol "nocreate" indent
		| value_to_eol "include" word indent
		| select_to_eol "missingok" /(no)?missingok/ indent
		| select_to_eol "compress" /(no)?compress/ indent
		| select_to_eol "delaycompress" /(no)?delaycompress/ indent
		| select_to_eol "ifempty" /(not)?ifempty/ indent
		| flag_to_eol "sharedscripts" indent
		| value_to_eol "size" word indent
		| value_to_eol "tabooext" word indent
		| value_to_eol "olddir" word indent
		| flag_to_eol "noolddir" indent
		| value_to_eol "mail" word indent
		| flag_to_eol "mailfirst" indent
		| flag_to_eol "maillast" indent
		| flag_to_eol "nomail" indent
		| value_to_eol "errors" word indent
		| value_to_eol "extension" word indent


   (* Define hooks *)


   let hook_lines = store ( ( /.*/ . "\n") - /[ \t]*endscript[ \t]*\n/ )* 
   
   let hook_func (func_type:string) = [
       del /[ \t]*/ "\t" . key func_type . eol .
       hook_lines .
       del /[ \t]*endscript\n/ "\tendscript\n" ]

   let hooks = hook_func "postrotate"
             | hook_func "prerotate"

   (* Define rule *)

   let body = Util.del_str "{\n"
                       . ( comment "\t" | attrs "\t" | hooks | empty )*
                       . Util.del_str "}\n"

   let rule = 
     [ label "rule" . 
         [ label "file" . store word ] .
	 [ del /[ \t]+/ " " . label "file" . store word ]* .
	 del /[ \t\n]*/ " " . body ]

   let lns = ( comment "" | empty | attrs "" | rule )*

   let filter = incl "/etc/logrotate.d/*"
              . incl "/etc/logrotate.conf"
	      . Util.stdexcl

   let xfm = transform lns filter

