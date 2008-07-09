module Pass_empty_put =

  let eol = del "\n" "\n"
  let lns = [ key /[a-z]+/ . del /[ \t]+/ " " . store /[a-z]+/ . eol ]*

  test lns put "" after
    set "entry" "value"
  = "entry value\n"
