(*
Module: Test_Build
  Provides unit tests and examples for the <Build> lens.
*)

module Test_Build =

(************************************************************************
 * Group:               GENERIC CONSTRUCTIONS
 ************************************************************************)

(* View: brackets
    Test brackets *)
let brackets = [ Build.brackets Sep.lbracket Sep.rbracket  (key Rx.word) ]

(* Test: brackets *)
test brackets get "(foo)" = { "foo" }


(************************************************************************
 * Group:             LIST CONSTRUCTIONS
 ************************************************************************)

(* View: list *)
let list = Build.list [ key Rx.word ] Sep.space

(* Test: list *)
test list get "foo bar baz" = { "foo" } { "bar" } { "baz" }

(* Test: list *)
test list get "foo" = * 

(* View: opt_list *)
let opt_list = Build.opt_list [ key Rx.word ] Sep.space

(* Test: opt_list *)
test opt_list get "foo bar baz" = { "foo" } { "bar" } { "baz" }


(************************************************************************
 * Group:                   LABEL OPERATIONS
 ************************************************************************)

(* View: xchg *)
let xchg = [ Build.xchg Rx.space " " "space" ]

(* Test: xchg *)
test xchg get " \t " = { "space" }

(* View: xchgs *)
let xchgs = [ Build.xchgs " " "space" ]

(* Test: xchgs *)
test xchgs get " " = { "space" }


(************************************************************************
 * Group:                   SUBNODE CONSTRUCTIONS
 ************************************************************************)

(* View: key_value_line *)
let key_value_line = Build.key_value_line Rx.word Sep.equal (store Rx.word)

(* Test: key_value_line *)
test key_value_line get "foo=bar\n" = { "foo" = "bar" }

(* View: key_value_line_comment *)
let key_value_line_comment = Build.key_value_line_comment Rx.word
                             Sep.equal (store Rx.word) Util.comment

(* Test: key_value_line_comment *)
test key_value_line_comment get "foo=bar # comment\n" =
    { "foo" = "bar" { "#comment" = "comment" } }

(* View: key_value *)
let key_value = Build.key_value Rx.word Sep.equal (store Rx.word)

(* Test: key_value *)
test key_value get "foo=bar" = { "foo" = "bar" }

(* View: key_ws_value *)
let key_ws_value = Build.key_ws_value Rx.word

(* Test: key_ws_value *)
test key_ws_value get "foo bar\n" = { "foo" = "bar" }

(* View: flag *)
let flag = Build.flag Rx.word

(* Test: flag *)
test flag get "foo" = { "foo" }

(* View: flag_line *)
let flag_line = Build.flag_line Rx.word

(* Test: flag_line *)
test flag_line get "foo\n" = { "foo" }


(************************************************************************
 * Group:                   BLOCK CONSTRUCTIONS
 ************************************************************************)

(* View: block_entry
    The block entry used for testing *)
let block_entry = Build.key_value "test" Sep.equal (store Rx.word)

(* View: block
    The block used for testing *)
let block = Build.block block_entry

(* Test: block
     Simple test for <block> *)
test block get " {test=1}" =
  { "test" = "1" }

(* Test: block
     Simple test for <block> with newlines *)
test block get " {\n test=1\n}" =
  { "test" = "1" }

(* Test: block
     Simple test for <block> two indented entries *)
test block get " {\n test=1 \n  test=2 \n}" =
  { "test" = "1" }
  { "test" = "2" }

(* Test: block
     Test <block> with a comment *)
test block get " { # This is a comment\n}" =
  { "#comment" = "This is a comment" }

(* Test: block
     Test <block> with comments and newlines *)
test block get " { # This is a comment\n# Another comment\n}" =
  { "#comment" = "This is a comment" }
  { "#comment" = "Another comment" }

(* Test: block
     Test defaults for blocks *)
test block put " { test=1 }" after
   set "/#comment" "a comment";
   rm "/test";
   set "/test" "2" =
  " { # a comment\ntest=2 }"

(* View: named_block
    The named block used for testing *)
let named_block = Build.named_block "foo" block_entry

(* Test: named_block
     Simple test for <named_block> *)
test named_block get "foo {test=1}\n" =
  { "foo" { "test" = "1" } }

(* View: logrotate_block
    A minimalistic logrotate block *)
let logrotate_block =
      let entry = [ key Rx.word ] 
   in let filename = [ label "file" . store /\/[^,#= \n\t{}]+/ ]
   in let filename_sep = del /[ \t\n]+/ " "
   in let filenames = Build.opt_list filename filename_sep
   in [ label "rule" . filenames . Build.block entry ]

(* Test: logrotate_block *)
test logrotate_block get "/var/log/wtmp\n/var/log/wtmp2\n{
   missingok
   monthly
}" =
  { "rule"
    { "file" = "/var/log/wtmp" }
    { "file" = "/var/log/wtmp2" }
    { "missingok" }
    { "monthly" }
  }


(************************************************************************
 * Group:               COMBINATORICS
 ************************************************************************)

(* View: combine_two
    A minimalistic combination lens *)
let combine_two =
     let entry (k:string) = [ key k ]
  in Build.combine_two (entry "a") (entry "b")

(* Test: combine_two 
     Should parse ab *)
test combine_two get "ab" = { "a" } { "b" }

(* Test: combine_two 
     Should parse ba *)
test combine_two get "ba" = { "b" } { "a" }

(* Test: combine_two 
     Should not parse a *)
test combine_two get "a" = *

(* Test: combine_two 
     Should not parse b *)
test combine_two get "b" = *

(* Test: combine_two 
     Should not parse aa *)
test combine_two get "aa" = *

(* Test: combine_two 
     Should not parse bb *)
test combine_two get "bb" = *
 

(* View: combine_two_opt
    A minimalistic optional combination lens *)
let combine_two_opt =
     let entry (k:string) = [ key k ]
  in Build.combine_two_opt (entry "a") (entry "b")

(* Test: combine_two_opt 
     Should parse ab *)
test combine_two_opt get "ab" = { "a" } { "b" }

(* Test: combine_two_opt 
     Should parse ba *)
test combine_two_opt get "ba" = { "b" } { "a" }

(* Test: combine_two_opt 
     Should parse a *)
test combine_two_opt get "a" = { "a" }

(* Test: combine_two_opt 
     Should parse b *)
test combine_two_opt get "b" = { "b" }

(* Test: combine_two_opt 
     Should not parse aa *)
test combine_two_opt get "aa" = *

(* Test: combine_two_opt 
     Should not parse bb *)
test combine_two_opt get "bb" = *


(* View: combine_three
    A minimalistic optional combination lens *)
let combine_three =
     let entry (k:string) = [ key k ]
  in Build.combine_three (entry "a") (entry "b") (entry "c")

(* Test: combine_three 
     Should not parse ab *)
test combine_three get "ab" = *

(* Test: combine_three 
     Should not parse ba *)
test combine_three get "ba" = *

(* Test: combine_three 
     Should not parse a *)
test combine_three get "a" = *

(* Test: combine_three 
     Should not parse b *)
test combine_three get "b" = *

(* Test: combine_three 
     Should not parse aa *)
test combine_three get "aa" = *

(* Test: combine_three 
     Should not parse bbc *)
test combine_three get "bbc" = *

(* Test: combine_three 
     Should parse abc *)
test combine_three get "abc" = { "a" } { "b" } { "c" }

(* Test: combine_three 
     Should parse cab *)
test combine_three get "cab" = { "c" } { "a" } { "b" }


(* View: combine_three_opt
    A minimalistic optional combination lens *)
let combine_three_opt =
     let entry (k:string) = [ key k ]
  in Build.combine_three_opt (entry "a") (entry "b") (entry "c")

(* Test: combine_three_opt 
     Should parse ab *)
test combine_three_opt get "ab" = { "a" } { "b" }

(* Test: combine_three_opt 
     Should parse ba *)
test combine_three_opt get "ba" = { "b" } { "a" }

(* Test: combine_three_opt 
     Should parse a *)
test combine_three_opt get "a" = { "a" }

(* Test: combine_three_opt 
     Should parse b *)
test combine_three_opt get "b" = { "b" }

(* Test: combine_three_opt 
     Should not parse aa *)
test combine_three_opt get "aa" = *

(* Test: combine_three_opt 
     Should not parse bbc *)
test combine_three_opt get "bbc" = *

(* Test: combine_three_opt 
     Should parse abc *)
test combine_three_opt get "abc" = { "a" } { "b" } { "c" }

(* Test: combine_three_opt 
     Should parse cab *)
test combine_three_opt get "cab" = { "c" } { "a" } { "b" }
