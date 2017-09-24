module Cont =

  (* An experiment in handling empty lines and comments in httpd.conf and
   * similar. What makes this challenging is that httpd.conf allows
   * continuation lines; the markers for continuation lines (backslash +
   * newline) needs to be treated like any other whitespace.  *)

  (* The continuation sequence that indicates that we should consider the
   * next line part of the current line *)
  let cont = /\\\\\r?\n/

  (* Whitespace within a line: space, tab, and the continuation sequence *)
  let ws = /[ \t]/ | cont

  (* Any possible character - '.' does not match \n *)
  let any = /(.|\n)/

  (* Newline sequence - both for Unix and DOS newlines *)
  let nl = /\r?\n/

  (* Whitespace at the end of a line *)
  let eol = del (ws* . nl) "\n"

  (* A complete line that is either just whitespace or a comment that only
   * contains whitespace *)
  let empty = [ del (ws* . /#?/ . ws* . nl) "\n" ]

  (* A comment that is not just whitespace. We define it in terms of the
   * things that are not allowed as part of such a comment:
   *   1) Starts with whitespace or newline
   *   2) Ends with whitespace, a backslash or \r
   *   3) Unescaped newlines
   *)
  let comment =
    let comment_start = del (ws* . "#" . ws* ) "# " in
    let unesc_eol = /[^\]/ . nl in
    (* As a complement, the above criteria can be written as
       let line = any* -  ((ws|nl) . any*
                          | any* . (ws|/[\r\\]/)
                          | any* . unesc_eol . any* )? in
     * Printing this out with 'print_regexp line' and simplifying it while
     * checking for equality with the ruby-fa bindings, we can write this
     * as follows: *)
    let w = /[^\t\n\r \\]/ in
    let r = /[\r\\]/ in
    let s = /[\t\r ]/ in
    let b = "\\\\" in
    let t = /[\t\n\r ]/ in
    let line = ((r . s* . w|w|r) . (s|w)* . (b . (t? . (s|w)* ))*|(r.s* )?).w.(s*.w)* in
    [ label "#comment" . comment_start . store line . eol ]

  let lns = (comment|empty)*

  test [eol] get " \n" = { }
  test [eol] get " \t\n" = { }
  test [eol] get "\\\n\n" = { }


  test lns get "#  \\\r\n \t \\\n\n" = { }
  test lns get "#  x\n" = { "#comment" = "x" }
  test lns get "#  x\\\n\n" = { "#comment" = "x" }
  test lns get "#  \\\r\n \tx \\\n\n" = { "#comment" = "x" }
  test lns get "  \t\\\n# x\n" = { "#comment" = "x" }
  test lns get "# word\\\n word  \n" = { "#comment" = "word\\\n word" }
  (* Not valid as it is an incomplete 'line' *)
  test lns get "# x\\\n" = *
