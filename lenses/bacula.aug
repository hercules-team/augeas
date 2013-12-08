(*
Module: Bacula
  Parses: /etc/bacula/*.conf

Author: Domen Kožar <domen@dev.si>

About: Reference
  See "man bacula"

About: License
  This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   See <lns>.

About: Configuration files
   This lens applies to /etc/bacula/*.conf.

About: Examples
   The <test_bacula.aug> file contains various examples and tests.
*)


module Bacula =
   autoload xfm

   let indent = Util.del_opt_ws "\t"
   let equal = del /[ \t]*=[ \t]*/ " = "
   let key_name = /[a-zA-Z][a-zA-Z ]+[a-zA-Z]/

   let re_value = /[^}"#\n\t; ]([^}"#\n;]*[^}"#\n\t; ])?/
   let val_quote = [square /"/ (store (/([^"#\n;]|\\")+/)) . [store (/ / . re_value)]?]
   let val = [label "" . store re_value]

   let keyvalue = key key_name . equal . (val|val_quote)
   let include = label "@include" . Util.del_str "@" . store /[^#\t\n@};]+[^# \t\n@};]/

   let semicolon = del /([ \t]*;)?/ ""
   let eol = del /[ \t]*(;|(#[ \t]*)?\n)/ "\n"
   let comment_or_eol = Util.comment_eol | eol
   let comment_or_semicolon = Util.comment_eol | semicolon

   let line (sto:lens) = [ sto . comment_or_eol ]
   let line_noeol (sto:lens) = [ sto . comment_or_semicolon ]

   let rec block =
        let entry = Util.empty | (indent . (line keyvalue | line include | block))
     in let entry_noindent = line keyvalue | line include | block
     in let entry_noindent_noeol = line_noeol keyvalue | line_noeol include | block
     in let entry_noeol = indent . entry_noindent_noeol
     in [ label "@block" . store /[a-zA-Z]+/
        . Build.block_generic
            entry                      (* entry *)
            entry_noindent             (* entry_noindent *)
            entry_noeol                (* entry_noeol *)
            entry_noindent_noeol       (* entry_noindent_noeol *) 
            Util.comment               (* comment *)
            Util.comment_noindent      (* comment_noindent *)
            /[ \t\n]*\{[ \t\n]*/       (* ldelim_re *)
            Build.block_rdelim_re      (* rdelim_re *) 
            " {\n\t"                   (* ldelim_default *) 
            Build.block_rdelim_default (* rdelim_default *) 
        ]

   let statement = Util.indent . (line keyvalue | line include | block)

   let lns = (statement|Util.empty|Util.comment)*

   let filter = incl "/etc/bacula/*.conf"
              . Util.stdexcl

   let xfm = transform lns filter
