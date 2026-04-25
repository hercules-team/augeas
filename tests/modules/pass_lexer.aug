module Pass_Lexer =

  (* Some tests for corner cases for the lexer; they will all lead to
   * syntax errors if we are not lexing correctly *)

  let s1 = "\\"
  let s2 = "\
"

  let r1 = /\\\\/

  let slash = "/" (* Just here to cause trouble if the lexer does not
                   * properly terminate the above expressions *)
