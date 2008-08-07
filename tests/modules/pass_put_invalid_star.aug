module Pass_put_invalid_star =

  let lns = [ key /[a-z]+/ . del "=" "=" . store /[0-9]+/ . del "\n" "\n" ]*

  test lns put "a=1\nb=2\n" after
    set "c2" "3"
  = *

  test lns put "a=1\n" after
    insb "x1" "a" ;
    set "x1" "1"
  = *



