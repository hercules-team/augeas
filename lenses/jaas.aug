(* Module Jaas *)
(* Author: Simon Vocella <voxsim@gmail.com> *)

module Jaas =

autoload xfm

let space_equal = del (/[ \t]*/ . "=" . /[ \t]*/) (" = ")
let lbrace = del (/[ \t\n]*/ . "{") "{"
let rbrace = del ("};") "};"
let word = /[A-Za-z0-9_.-]+/

let value_re =
        let value_squote = /'[^\n']*'/
        in let value_squote_2 = /'[^\n']*';/
        in let value_dquote = /"[^\n"]*"/
        in let value_dquote_2 = /"[^\n"]*";/
        in value_squote | value_squote_2 | value_dquote | value_dquote_2

let moduleOption = [Util.del_opt_ws "" . key word . space_equal . (store value_re . Util.comment_or_eol)]
let flag = [label "flag" . (store word . Util.eol) . moduleOption*]
let loginModuleClass = [Util.del_opt_ws "" . label "loginModuleClass" . (store word . Util.del_ws_spc) . flag]

let content = (Util.empty | Util.comment_c_style | Util.comment_multiline | loginModuleClass)*
let loginModule = [Util.del_opt_ws "" . label "login" . (store word . lbrace) . (content . rbrace)]
let lns = (Util.empty | Util.comment_c_style | Util.comment_multiline | loginModule)*
let filter = incl "/opt/shibboleth-idp/conf/login.config"
let xfm = transform lns filter
