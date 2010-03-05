module Approx =
  autoload xfm

  let eol = Util.eol
  let indent = Util.indent
  let key_re = /\$?[A-Za-z0-9_.-]+/
  let sep = /[ \t]+/
  let value_re = /[^ \t\n](.*[^ \t\n])?/

  let comment = [ indent . label "#comment" . del /[#;][ \t]*/ "# "
        . store /([^ \t\n].*[^ \t\n]|[^ \t\n])/ . eol ]

  let empty = Util.empty

  let kv = [ indent . key key_re . del sep " " . store value_re . eol ]

  let lns = (empty | comment | kv) *

  let filter = incl "/etc/approx/approx.conf"
  let xfm = transform lns filter
