module Fail_Nocase_Union =

let lns = [ key /[a-z]+/i ] | [ key "UPPER" ]
