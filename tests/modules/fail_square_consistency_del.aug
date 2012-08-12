module Fail_square_consistency_del =

let left = del /[ab]/ "a"
let right = del /[ab]/ "b"
let body = del "x" "x"
let s = square left body right
