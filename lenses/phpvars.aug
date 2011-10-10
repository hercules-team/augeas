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

let sep_php    = del /<\?php[ \t]*\n/ "<?php\n"
let sep_eq     = del /[ \n]*=/ " ="
let sep_spc    = del /[ \n]+/ " "
let sep_dollar = del /\$/ "$"
let sep_scl    = del /[ \t]*;/ ";"

let chr_blank = /[ \t]/
let chr_nblank = /[^ \t\n]/
let chr_any    = /./
let chr_star   = /\*/
let chr_nstar  = /[^\* \t\n]/
let chr_slash  = /\//
let chr_nslash = /[^\/ \t\n]/

let sto_to_scl = store /([^ \t\n].*[^ \t\n;]|[^ \t\n;])/
let sto_to_eol = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/

(************************************************************************
 *                              COMMENTS
 *************************************************************************)

let comment_re = chr_nblank
               | ( chr_nblank . chr_any*
                   . ( chr_star  . chr_nslash
                     | chr_nstar . chr_slash
                     | chr_nstar . chr_nslash
                     | chr_blank . chr_nblank ) )

let comment_first_line
               = [ indent
                 . seq "#comment"
                 . store comment_re
                  ]
let comment_other_line
               = [ del /[ \t]*\n[ \t\n]*/ "\n"
                 . seq "#comment"
                 . store comment_re
                  ]
let comment_end
               = del /[ \t\n]*/ "" . del (chr_star . chr_slash) "*/"

let comment_extended
                 = [ indent
                 . del (chr_slash . chr_star) "/*"
                 . label "#comment"
                 . counter "#comment"
                 . ( (comment_first_line . comment_other_line+)
                   | comment_first_line?)
                 . comment_end
                 . eol ]

let comment_inline
                 = [ indent
                 . del (chr_slash . chr_slash) "//"
                 . label "#inline"
                 . indent
                 . sto_to_eol
                 . eol ]

let comment      = comment_extended | comment_inline

(************************************************************************
 *                               ENTRIES
 *************************************************************************)

let global     = [ key "global"
                 . sep_spc
                 . sep_dollar
                 . sto_to_scl
                 . sep_scl
                 . eol ]

let variable_re
               = /\$[][A-Za-z0-9'_-]+/
let variable   = [ key variable_re
                 . sep_eq
                 . sep_spc
                 . sto_to_scl
                 . sep_scl
                 . eol ]

let include    = [ key "@include"
                 . sep_spc
                 . sto_to_scl
                 . sep_scl
               .   eol ]

let entry      = global|variable|include

(************************************************************************
 *                                LENS
 *************************************************************************)

let lns        = sep_php . (empty|comment|entry)*

let filter     = incl "/etc/squirrelmail/config.php"
               . Util.stdexcl

let xfm        = transform lns filter
