module Fail_put_bad_value =
  
  let lns = [ key /a/ . del /=/ "=" . store /[0-9]+/ ]

  test lns put "a=20" after set "a" "foo" = ?
