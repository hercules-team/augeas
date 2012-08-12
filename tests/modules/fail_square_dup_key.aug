module Fail_square_dup_key =

let left = key "a"
let right = del "a" "a"
let body = key "a"
let s = square left body right
