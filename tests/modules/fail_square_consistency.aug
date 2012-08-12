module Fail_square_consistency =

let left = key "a"
let right = del "b" "b"
let body = del "x" "x"
let s = square left body right
