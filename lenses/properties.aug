(* Augeas module for editing tomcat properties files
 Author: Craig Dunn <craig@craigdunn.org>

 FIXME: Doesn't cover everything that's legal in Java properties yet
        - multiline
        - "!" comments
        - key:value syntax
*)


module Properties =
  (* Define some basic primitives *)
  let empty        = Util.empty
  let eol          = Util.eol
  let sepch        = del /[ \t]*=[ \t]*/ "="
  let value_to_eol = /[^ \t\n](.*[^ \t\n])?/
  let indent       = Util.indent
  let entry        = /[A-Za-z][A-Za-z0-9.]+/

  (* define comments and properties*)
  let comment     = Util.comment
  let property    = [ indent . key entry . sepch . store value_to_eol . eol ]

  (* setup our lens and filter*)
  let lns         = ( property | empty | comment  ) *
