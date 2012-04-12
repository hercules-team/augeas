(* Augeas module for editing Java properties files
 Author: Craig Dunn <craig@craigdunn.org>

  Limitations:
   - doesn't support \ alone on a line
   - doesn't support ' ' as a separator
   - values are not unescaped
   - multi-line properties are broken down by line, and can't be replaced with a single line

  See format info: http://docs.oracle.com/javase/6/docs/api/java/util/Properties.html#load(java.io.Reader)
*)


module Properties =
  (* Define some basic primitives *)
  let empty            = Util.empty
  let eol              = Util.eol
  let hard_eol         = del "\n" "\n"
  let sepch            = del /[ \t]*(=|:)/ "="
  let sepch_opt        = del /[ \t]*(=|:)?[ \t]*/ "="
  let value_to_eol     = store /([^ \t\n][^\n]*[^ \t\n\\]|[^ \t\n\\])/
  let value_to_bs      = store /([^ \t\n][^\n]*[^\\]|[^ \t\n\\])/
  let indent           = Util.indent
  let backslash        = del /[\\][ \t]*\n/ "\\\n"
  let entry            = /([^ \t\n:=\/!#]|[\\]:|[\\]=)+/

  let multi_line_entry =
      [ indent . value_to_bs . backslash ] + .
      [ indent . value_to_eol . eol ] . value " < multi > "

  (* define comments and properties*)
  let bang_comment     = [ label "!comment" . del /[ \t]*![ \t]*/ "! " . store /([^ \t\n].*[^ \t\n]|[^ \t\n])/ . eol ]
  let comment          = ( Util.comment | bang_comment )
  let property         = [ indent . key entry . sepch . ( multi_line_entry | indent . value_to_eol . eol ) ]
  let empty_property   = [ indent . key entry . sepch_opt . hard_eol ]
  let empty_key        = [ sepch . ( multi_line_entry | indent . value_to_eol . eol ) ]

  (* setup our lens and filter*)
  let lns              = ( empty | comment | property | empty_property | empty_key ) *
