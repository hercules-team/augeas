module Pass_ins_test =

  let eol = del "\n" "\n"
  let word = /[a-z0-9]+/
  let lns = [ key word . del /[ \t]+/ " " . store word . eol ]*

  let s = "key value1\nkey value2\n"
  let t = "key value1\nnewkey newvalue\nkey value2\n"

  test lns put s after
    insa "newkey" "key[1]";
    set "newkey" "newvalue"
  = t

  test lns put s after
    insb "newkey" "key[2]";
    set "newkey" "newvalue"
  = t
