(* PHP module for Augeas                      *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)

module PHP =
  autoload xfm

(* PHP is a standard INI file *)


(* Importe useful primitives *)
let comment = IniFile.comment
let empty   = IniFile.empty
let eol     = IniFile.eol


(*
  We have to remove the keyword "section" from possible entry keywords 
  otherwise it would lead to an ambiguity with the "section" label
  since PHP allows entries outside of sections.
*)
let entry  = IniFile.entry ( /[a-z][a-z0-9\._-]+/ - /section/ )


let title_label
           = label "section"
             .Util.del_str "[" . store /[^]]+/
             . Util.del_str "]". eol

let record = [ title_label
             . (entry | comment | empty)* ]

let lns    = ( IniFile.comment | IniFile.empty | entry )* . record*

let filter = (incl "/etc/php*/*/php.ini")
             . Util.stdexcl

let xfm = transform lns filter

