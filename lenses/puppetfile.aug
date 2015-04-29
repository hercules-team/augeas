(*
Module: Puppetfile
  Parses libarian-puppet's Puppetfile format

Author: Raphael Pinson <raphael.pinson@camptocamp.com>

About: Reference
  See https://github.com/rodjek/librarian-puppet

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to Puppetfiles.

About: Examples
   The <Test_Puppetfile> file contains various examples and tests.
*)

module Puppetfile =

(* View: comma
     a comma, optionally preceded or followed by spaces or newlines *)
let comma = del /[ \t\n]*,[ \t\n]*/ ", "

(* View: forge
     a forge entry *)
let forge = [ Util.indent . key "forge" . Sep.space . Quote.any . Util.eol ]

(* View: metadata
     a metadata entry *)
let metadata = [ Util.indent . key "metadata" . Util.eol ]

(* View: mod
     a module entry, with optional version and options *)
let mod =
     let mod_name = Quote.do_quote (store (Rx.word . /[\/-]/ . Rx.word))
  in let version = [ label "@version" . Quote.do_quote (store /[^:\n]+/) ]
  in let opt = [ Util.del_str ":" . key Rx.word . del /[ \t]*=>[ \t]*/ " => "
               . Quote.do_quote (store /[^,\n]*/) ]
  in let opts = Build.opt_list opt comma
  in [ Util.indent . Util.del_str "mod" . seq "mod" . Sep.space . mod_name
     . (comma . version)?
     . (comma . opts)?
     . Util.eol ] 

(* View: lns
     the Puppetfile lens *)
let lns = (Util.empty | Util.comment | forge | metadata | mod)*
