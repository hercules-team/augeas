module Pass_switch_value_regexp =

  let lns =  ([ key /u/ ] . [ key /b/ . store /[0-9]+/ ])
           | ([ key /v/ ] . [ key /b/ . store /[a-z]+/ ])

  test lns get "ub42" = { "u" } { "b" = "42" }
  test lns get "vbxy" = { "v" } { "b" = "xy" }
  test lns get "ubxy" = *
  test lns get "vb42" = *
