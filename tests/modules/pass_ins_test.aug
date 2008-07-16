module Pass_ins_test =

  let eol = del "\n" "\n"
  let word = /[a-z0-9]+/
  let sep = del /[ \t]+/ " "
  let lns = [ key word . sep . store word . eol ]*

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

  test lns put s after
    insb "newkey" "key[1]";
    set "newkey" "newvalue"
  = "newkey newvalue\n" . s

  (* Now test insertion inside the tree *)

  let lns2 = [ key word . eol . lns ]

  let s2 = "outer\n" . s
  let t2 = "outer\n" . t

  test lns2 put s2 after
    insa "newkey" "outer/key[1]";
    set "outer/newkey" "newvalue"
  = t2

  test lns2 put s2 after
    insb "newkey" "outer/key[2]";
    set "outer/newkey" "newvalue"
  = t2


  test lns2 put s2 after
    insb "newkey" "outer/key[1]";
    set "outer/newkey" "newvalue"
  = "outer\nnewkey newvalue\n" . s

