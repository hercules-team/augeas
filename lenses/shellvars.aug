(* Generic lens for shell-script config files like the ones found *)
(* in /etc/sysconfig                                              *)
module Shellvars =

  let eol = Util.eol

  let key_re = /[A-Za-z0-9_]+(\[[0-9]+\])?/
  let eq = Util.del_str "="
  let comment = [ del /(#.*)?[ \t]*\n/ "# \n" ]

  let char  = /[^() '"\t\n]|\\\\"/   
  let dquot = /"([^"\\\n]|\\\\.)*"/                    (* " Emacs, relax *)
  let squot = /'[^'\n]*'/

  (* Array values of the form '(val1 val2 val3)'. We do not handle empty *)
  (* arrays here because of typechecking headaches. Instead, they are    *)
  (* treated as a simple value                                           *)
  let array =
    let array_value = store (char+ | dquot) in
    del "(" "(" . counter "values" .
      [ seq "values" . array_value ] .
      [ del /[ \t]+/ " " . seq "values" . array_value ] *
      . del ")" ")"
  
  (* Treat an empty list () as a value '()'; that's not quite correct *)
  (* but fairly close.                                                *)
  let simple_value = 
    let empty_array = /\([ \t]*\)/ in
   store (char* | dquot | squot | empty_array)

  let kv = [ key key_re . eq . (simple_value | array) . eol ]

  let source = 
    [ 
      del /\.|source/ "." . label ".source" . 
      Util.del_ws_spc . store /[^= \t\n]+/ . eol 
    ]

  let lns = (comment | source | kv) *

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
