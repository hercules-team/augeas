(* Module: Nginx
   Nginx module for Augeas

Authors: Ian Berry <iberry@barracuda.com>
         Raphael Pinson <raphael.pinson@camptocamp.com>

About: Reference

   This module was built to support a limited subset of nginx
   configuration syntax. It works fine with simple blocks and
   field/value lines.

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to /etc/nginx/nginx.conf. See <filter>.

About: Examples
   The <Test_Nginx> file contains various examples and tests.

About: TODO
   * Convert statement keyworks for a regex
   * Support more advanced block syntax (location)
*)

module Nginx =

autoload xfm

(* Variable: block_re
     The keywords reserved for block entries *)
let block_re = "http" | "events"
             | "server" | "upstream"
             | "location" | "if"

(* View: simple
     A simple entry *)
let simple = 
     let kw = Rx.word - block_re
  in let sto = store /[^ \t\n;][^;]*/ . Sep.semicolon
  in Build.key_value_line_comment
     kw Sep.space sto Util.comment_eol

(* View: block
     A block containing <simple> entries *)
let block =
     let entry = Util.indent . simple
  in [ key block_re
     . Build.block_newlines entry Util.comment
     . Util.eol ]

(* View: lns *)
let lns = ( Util.comment | Util.empty | simple | block )*

(* Variable: filter *)
let filter = incl "/etc/nginx/nginx.conf"
           . incl "/usr/portage/www-servers/nginx/files/nginx.conf"

let xfm = transform lns filter
