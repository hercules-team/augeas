(* Demonstrate how quotes can be stripped from values.            *)
(* Values can be enclosed in single or double quotes, if they     *)
(* contain spaces they _have_ to be enclosed in quotes. Since     *)
(* everything's regular, we can't actually match a closing quote  *)
(* to the opening quote, so "hello' will be accepted, too.        *)

module Pass_strip_quotes =

  let nuttin = del /(""|'')?/ "''"
  let bare = del /["']?/ "" . store /[a-zA-Z0-9]+/ . del /["']?/ ""
  let quoted = del /["']/ "'" . store /.*[ \t].*/ . del /["']/ "'"

  let lns = [ label "foo" . bare ]
    | [ label "foo" . quoted ]
    | [ label "foo" . nuttin ]

  test lns get "'hello'" = { "foo" = "hello" }
  test lns get "'hello world'" = { "foo" = "hello world" }

  let hw = "'hello world'"
  let set_hw = set "/foo" "hello world"
  test lns put "hello" after set_hw = hw
  test lns put "'hello world'" after set "/foo" "hello" = "'hello'"

  test lns put "" after set_hw = hw
  test lns put "\"\"" after set_hw = hw
