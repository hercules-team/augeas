module Pass_regexp_minus =

  let word = /[a-z]+/
  let no_baseurl = word - "baseurl"

  let eq = del "=" "="

  let l1 = [ key no_baseurl . eq . store /[a-z]+/ ]
  let l2 = [ key "baseurl" . eq . store /[0-9]+/ ]

  let lns = ((l1 | l2) . del "\n" "\n")*

  test lns get "foo=abc\nbaseurl=123\n" = 
    { "foo" = "abc" }
    { "baseurl" = "123" }

  test lns get "baseurl=abc\n" = *      
  

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
