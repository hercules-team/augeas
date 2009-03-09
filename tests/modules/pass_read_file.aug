module Pass_read_file =

(* This is a roundabout way to test that Sys.getenv and Sys.read_file *)
(* work. Since we don't have a generic unit testing facility, we need *)
(* to phrase things in terms of a lens test.                          *)

let fname = (Sys.getenv "abs_top_srcdir") . "/tests/root/pairs.txt"
let str = (Sys.read_file fname)
let lns = [ key /[a-z0-9]*/ . del /[ \t]*=[ \t]*/ "="
    . store /[^ \t\n]*/ . del /\n/ "\n" ] *

test lns get str =
  { "key1" = "value1" }
  { "key2" = "value2" }
  { "key3" = "value3" }
