module Fail_square_lens_type =

let left = [ key "a" ]
let right = [ key "a" ]
let body = del "x" "x"
let s = square left body right
