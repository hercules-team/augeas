module Pass_empty_regexp =

let l = [ del // "" . key /[a-z]+/ ]

test l get "abc" = { "abc" }
