(* Parsing yum's config files *)
module Yum =
  let eol = Util.del_str "\n"

  let key_re = /[A-Za-z0-9_-]+/
  let eq = del /[ \t]*=[ \t]*/ "="
  let secname = /[A-Za-z0-9]+/
  let value = /[^ \t][^\n]*/

  (* We really need to allow comments starting with REM and rem but that     *)
  (* leads to ambiguities with keys 'rem=' and 'REM=' The regular expression *)
  (* to do that cleanly is somewhat annoying to craft by hand; we'd need to  *)
  (* define KEY as /[A-Za-z0-9]+/ - "REM" - "rem"                            *)
  let comment = [ del /[ \t]*(#.*)?\n/ "# \n" ]

  let kv = [ key key_re . eq . store value . eol ]

  let section = [ 
                  Util.del_str "[" . key secname . Util.del_str "]" . eol .
                  ( comment | kv ) *
                ]

  let lns = (comment) * . (section) *

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
