module Pass_nocase =

let lns1 =
  let re = /[a-z]+/i - "Key" in
  [ label "1" . store re ] | [ label "2" . store "Key" ]

test lns1 get "Key" = { "2" = "Key" }
test lns1 get "key" = { "1" = "key" }
test lns1 get "KEY" = { "1" = "KEY" }
test lns1 get "KeY" = { "1" = "KeY" }

let lns2 =
  let re = /[A-Za-z]+/ - /Key/i in
  [ label "1" . store re ] | [ label "2" . store /Key/i ]

test lns2 get "Key" = { "2" = "Key" }
test lns2 get "key" = { "2" = "key" }
test lns2 get "KEY" = { "2" = "KEY" }
test lns2 get "KeY" = { "2" = "KeY" }
