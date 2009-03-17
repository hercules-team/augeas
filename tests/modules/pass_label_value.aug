module Pass_label_value =

let l = [ label "label" . value "value" ]

test l get "" = { "label" = "value" }

test l put "" after rm "/foo" = ""

let word = /[^ \t\n]+/
let ws = del /[ \t]+/ " "
let chain = [ key "RewriteCond" . ws .
              [ label "eq" . store word ] . ws . store word .
              ([ label "chain_as" . ws . del "[OR]" "[OR]" . value "or"]
              |[ label "chain_as" . value "and" ]) ]

test chain get "RewriteCond %{var} val [OR]" =
  { "RewriteCond" = "val"
    { "eq" = "%{var}" }
    { "chain_as" = "or" } }

test chain get "RewriteCond %{var} lue" =
  { "RewriteCond" = "lue"
    { "eq" = "%{var}" }
    { "chain_as" = "and" } }

test chain put "RewriteCond %{var} val [OR]" after
  set "/RewriteCond/chain_as" "and" = "RewriteCond %{var} val"
