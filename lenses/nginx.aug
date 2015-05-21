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
let block_re = "http" | "events" | "server" | "mail" | "stream"

(* All block keywords, including the ones we treat specially *)
let block_re_all = block_re | "if" | "location" | "geo" | "map"
  | "split_clients" | "upstream"

(* View: simple
     A simple entry *)
let simple =
     let kw = Rx.word - block_re_all
  in let sto = store /[^ \t\n;][^;]*/ . Sep.semicolon
  in [ Util.indent . key kw . Sep.space . sto . (Util.eol|Util.comment_eol) ]

let arg (name:string) (rx:regexp) =
  [ label name . Sep.space . store rx ]

(* Match any argument (as much as possible) *)
let any_rx =
  let bare_rx = /[^" \t\n{][^ \t\n{]*/ in
  let dquote_rx = /"([^\"]|\\.)*"/ in
  bare_rx | dquote_rx

let any_arg (name:string) = arg name any_rx

(* 'if' conditions are enclosed in matching parens which we can't match
   precisely with a regular expression. Instead, we gobble up anything that
   doesn't contain an opening brace. That can of course lead to trouble if
   a condition actually contains an opening brace *)
let block_if = key "if"
             . arg "#cond" /\(([^ \t\n{]|[ \t\n][^{])*\)/

let block_location = key "location"
  . (arg "#comp" /=|~|~\*|\^~/)?
  . any_arg "#uri"

let block_geo = key "geo"
  . (any_arg "#address")?
  . any_arg "#geo"

let block_map = key "map"
  . any_arg "#source"
  . any_arg "#variable"

let block_split_clients = key "split_clients"
  . any_arg "#string"
  . any_arg "#variable"

let block_upstream = key "upstream"
  . any_arg "#name"

let block_head = key block_re
  | block_if
  | block_location
  | block_geo
  | block_map
  | block_split_clients
  | block_upstream

(* View: block
     A block containing <simple> entries *)
let block (entry : lens) =
  [ Util.indent . block_head
  . Build.block_newlines entry Util.comment
  . Util.eol ]

let rec directive = simple | block directive

(* View: lns *)
let lns = ( Util.comment | Util.empty | directive )*

(* Variable: filter *)
let filter = incl "/etc/nginx/nginx.conf"
           . incl "/etc/nginx/conf.d/*.conf"
           . incl "/usr/portage/www-servers/nginx/files/nginx.conf"

let xfm = transform lns filter
