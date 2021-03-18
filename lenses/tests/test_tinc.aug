module Test_tinc =

let lns = Tinc.lns

test lns get "Subnet = 10.1.4.5\n" = { "Subnet" = "10.1.4.5" }
test lns get "foo = bar\n" = { "foo" = "bar" }
test lns get "foo bar\n" = { "foo" = "bar" }
test lns get "foo  bar\n" = { "foo" = "bar" }

test lns get
"-----BEGIN RSA PUBLIC KEY-----
abcde
-----END RSA PUBLIC KEY-----" = { "#key" = "abcde" }

test lns get "foo = bar\nbar = baz\n" =
  { "foo" = "bar" }
  { "bar" = "baz" }

test lns get
"foo = bar

-----BEGIN RSA PUBLIC KEY-----
bar
-----END RSA PUBLIC KEY-----" =
  { "foo" = "bar" }
  {  }
  { "#key" = "bar" }


(*
test lns get
"-----BEGIN RSA PUBLIC KEY-----
foo
-----END RSA PUBLIC KEY-----

-----BEGIN RSA PUBLIC KEY-----
bar
-----END RSA PUBLIC KEY-----
" = ?
*)
