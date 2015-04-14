(* Module Jaas *)
(* Author: Simon Vocella <voxsim@gmail.com> *)
(* Updated by: Steve Shipway <steve@steveshipway.org> *)
(* Changes: allow comments within Modules, allow optionless flags, allow options without linebreaks, allow naked true/false options *)

module Jaas =

autoload xfm

let space_equal = del (/[ \t]*/ . "=" . /[ \t]*/) (" = ")
let lbrace = del (/[ \t\n]*/ . "{") "{"
let rbrace = del ("};") "};"
let word = /[A-Za-z0-9_.-]+/
let endflag = del ( /[ \t]*;/ ) ( ";" )

let value_re =
        let value_squote = /'[^\n']*'/
        in let value_dquote = /"[^\n"]*"/
        in let value_tf = /(true|false)/
        in value_squote | value_dquote | value_tf

let moduleOption = [ ( Util.del_opt_ws "" . key word . space_equal . (store value_re . Util.del_opt_ws "") | Util.empty | Util.comment_c_style | Util.comment_multiline  )]
let flag = [label "flag" . (store word . Util.del_opt_ws " " ) . moduleOption* . endflag ]
let loginModuleClass = [( Util.del_opt_ws "" . label "loginModuleClass" . (store word . Util.del_ws_spc) .  flag  | Util.empty | Util.comment_c_style | Util.comment_multiline ) ]

let content = (Util.empty | Util.comment_c_style | Util.comment_multiline | loginModuleClass)*
let loginModule = [Util.del_opt_ws "" . label "login" . (store word . lbrace) . (content . rbrace)]
let lns = (Util.empty | Util.comment_c_style | Util.comment_multiline | loginModule)*
let filter = incl "/opt/shibboleth-idp/conf/login.config"
let xfm = transform lns filter
