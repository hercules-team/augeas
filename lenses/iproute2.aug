module IPRoute2 =
  autoload xfm

  let empty   = [ del /[ \t]*#?[ \t]*\n/ "\n" ]
  let id = Rx.hex | Rx.integer
  let record = [ key id . del /[ \t]+/ "\t" . store /[a-zA-Z0-9\/-]+/ . Util.comment_or_eol ]

  let lns = ( empty | Util.comment | record ) *

  let filter = incl "/etc/iproute2/*"
             . incl "/usr/share/iproute2/*"
             . excl "/etc/iproute2/README"
             . Util.stdexcl

  let xfm = transform lns filter
