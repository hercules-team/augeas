module Pass_prefix_union =

let eol = del "\n" "\n"
let pair (k:regexp) (v:regexp) = [ key k . del "=" "=" . store v . eol ]

let lns = pair "a" "a" | pair "a" "a" . pair "b" "b"

test lns get "a=a\nb=b\n" = { "a" = "a" } { "b" = "b" }
