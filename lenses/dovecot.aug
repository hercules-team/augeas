(*
 * Module: Dovecot
 *     Parses dovecot configuration files
 *
 *  Copyright (c) 2013 Pluron, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * TODO: Support for multiline queries like in dict-sql.conf
 *
 *)

module Dovecot =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol           = Util.eol
let comment       = Util.comment
let empty         = Util.empty
let indent        = Util.indent
let eq            = del /[ \t]*=/ " ="
let any           = Rx.no_spaces

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let value         = any . (Rx.space . any)* 
let command_start = Util.del_str "!"
let commands      = /include|include_try/
let block_names   = /dict|userdb|passdb|protocol|service|plugin|namespace|map|fields|unix_listener|fifo_listener|inet_listener/
let keys          = Rx.word - (commands | block_names)
let block_args    = Sep.space . store /[A-Za-z0-9\/\\_-]+/

let entry         = [ indent . key keys. eq . (Sep.opt_space . store value)? . eol ]
let command       = [ command_start . key commands . Sep.space . store Rx.fspath . eol ]
let rec block     = [ indent . key block_names . block_args? . Build.block_newlines (entry|block) comment . eol ]

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns          = (comment|empty|entry|command|block)*

let filter       = incl "/etc/dovecot/dovecot.conf"
                 . (incl "/etc/dovecot/conf.d/*.conf")
                 . Util.stdexcl

let xfm          = transform lns filter
