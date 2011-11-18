(* Phpvars module for Augeas
 Author: Free Ekanayaka <free@64studio.com>

 Reference: PHP syntax

*)

module Phpvars =

   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol        = Util.eol
let indent     = Util.indent
let empty      = Util.empty

let open_php   = del /<\?(php)?[ \t]*\n/ "<?php\n"
let close_php  = del /([ \t]*(php)?\?>\n)?/ "php?>\n"
let sep_eq     = del /[ \t\n]*=[ \t\n]*/ " = "
let sep_opt_spc = Sep.opt_space
let sep_spc    = Sep.space
let sep_dollar = del /\$/ "$"
let sep_scl    = del /[ \t]*;/ ";"

let chr_blank = /[ \t]/
let chr_nblank = /[^ \t\n]/
let chr_any    = /./
let chr_star   = /\*/
let chr_nstar  = /[^* \t\n]/
let chr_slash  = /\//
let chr_nslash = /[^\/ \t\n]/

let sto_to_scl = store (/([^ \t\n].*[^ \t\n;]|[^ \t\n;])/ - /.*;[ \t]*\/\/.*/)
let sto_to_eol = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/

(************************************************************************
 *                              COMMENTS
 *************************************************************************)

let comment      = Util.comment_multiline | Util.comment_c_style

let eol_or_comment = eol | Util.comment_c_style

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let global     = [ key "global"
                 . sep_opt_spc
                 . sep_dollar
                 . sto_to_scl
                 . sep_scl
                 . eol_or_comment ]

let variable_re
               = /\$[][A-Za-z0-9'_-]+/
let variable   = [ key variable_re
                 . sep_eq
                 . sto_to_scl
                 . sep_scl
                 . eol_or_comment ]

let include    = [ key "@include"
                 . sep_opt_spc
                 . sto_to_scl
                 . sep_scl
               .   eol_or_comment ]

let entry      = global|variable|include

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = open_php . (empty|comment|entry)* . close_php

let filter     = incl "/etc/squirrelmail/config.php"
               . Util.stdexcl

let xfm        = transform lns filter
