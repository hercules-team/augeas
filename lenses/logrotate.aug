(* Logrotate module for Augeas                *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(* Patches from:                              *)
(*   Sean Millichamp <sean@bruenor.org>       *)
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

   let list_item = [ sep_spc . key /[^\/+,# \n\t{}]+/ ]
   let select_to_eol (kw:string) (select:regexp) (indent:string) = [ del /[ \t]*/ indent . label kw . store select . eol ]
   let value_to_eol (kw:string) (value:regexp) (indent:string )  = [ del /[ \t]*/ indent . key kw . sep_spc . store value . eol ]
   let flag_to_eol (kw:string) (indent:string)                   = [ del /[ \t]*/ indent . key kw . eol ]
   let list_to_eol (kw:string) (indent:string)                   = [ del /[ \t]*/ indent . key kw . list_item+ . eol ]


   (* Defaults *)

   let create (indent:string ) = [ del /[ \t]*/ indent . key "create" .
                       ( sep_spc . [ label "mode" . store num ] . sep_spc .
		       [ label "owner" . store word ] . sep_spc .
		       [ label "group" . store word ])?
		    . eol ]

   let tabooext (indent:string) = [ del /[ \t]*/ indent . key "tabooext" . ( sep_spc . store /\+/ )? . list_item+ . eol ]

   let attrs (indent:string) = select_to_eol "schedule" /(daily|weekly|monthly)/ indent
                | value_to_eol "rotate" num indent
		| create indent
		| flag_to_eol "nocreate" indent
		| value_to_eol "include" word indent
		| select_to_eol "missingok" /(no)?missingok/ indent
		| select_to_eol "compress" /(no)?compress/ indent
		| select_to_eol "delaycompress" /(no)?delaycompress/ indent
		| select_to_eol "ifempty" /(not)?ifempty/ indent
		| select_to_eol "sharedscripts" /(no)?sharedscripts/ indent
		| value_to_eol "size" word indent
		| tabooext indent
		| value_to_eol "olddir" word indent
		| flag_to_eol "noolddir" indent
		| value_to_eol "mail" word indent
		| flag_to_eol "mailfirst" indent
		| flag_to_eol "maillast" indent
		| flag_to_eol "nomail" indent
		| value_to_eol "errors" word indent
		| value_to_eol "extension" word indent
		| select_to_eol "dateext" /(no)?dateext/ indent
		| value_to_eol "compresscmd" word indent
		| value_to_eol "uncompresscmd" word indent
		| value_to_eol "compressext" word indent
		| list_to_eol "compressoptions" indent
		| select_to_eol "copy" /(no)?copy/ indent
		| select_to_eol "copytruncate" /(no)?copytruncate/ indent
		| value_to_eol "maxage" num indent
		| value_to_eol "minsize" num indent
		| select_to_eol "shred" /(no)?shred/ indent
		| value_to_eol "shredcycles" num indent
		| value_to_eol "start" num indent

   (* Define hooks *)


   let hook_lines = store ( ( /.*/ . "\n") - /[ \t]*endscript[ \t]*\n/ )* 
   
   let hook_func (func_type:string) = [
       del /[ \t]*/ "\t" . key func_type . eol .
       hook_lines .
       del /[ \t]*endscript\n/ "\tendscript\n" ]

   let hooks = hook_func "postrotate"
             | hook_func "prerotate"
             | hook_func "firstaction"
             | hook_func "lastaction"

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

