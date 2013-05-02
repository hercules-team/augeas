module IPRoute2 =
  autoload xfm

  let empty   = [ del /[ \t]*#?[ \t]*\n/ "\n" ]
  let record = [ store /[0-9]+/ . del /[ \t]+/ "\t" . key /[a-zA-Z0-9]+/ . Util.comment_or_eol ]

  let lns = ( empty | Util.comment | record ) *

  let xfm = transform lns (incl "/etc/iproute2/*" . Util.stdexcl)
