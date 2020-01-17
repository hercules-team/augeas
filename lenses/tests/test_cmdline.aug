module Test_cmdline =

let lns = Cmdline.lns

test lns get "foo\nbar" = *
test lns get "foo\n" = { "foo" }
test lns get "foo" = { "foo" }
test lns get "foo bar" = { "foo" } { "bar" }
test lns get "foo    bar" = { "foo" } { "bar" }
test lns get "foo=bar" = { "foo" = "bar" }
test lns get "foo=bar foo=baz" = { "foo" = "bar" } { "foo" = "baz" }
test lns get "foo bar=bar      quux baz=x" =
        { "foo" } { "bar" = "bar" } { "quux" } { "baz" = "x" }
test lns get "initrd=\linux\initrd.img-4.19.0-6-amd64 root=UUID=SOME_UUID rw" =
        { "initrd" = "\linux\initrd.img-4.19.0-6-amd64" } { "root" = "UUID=SOME_UUID" } { "rw" }

test lns put "" after set "foo" "bar" = "foo=bar"
test lns put "foo=bar" after rm "foo" = ""
test lns put "x=y foo=bar" after set "foo" "baz" = "x=y foo=baz"
test lns put "foo=bar foo=baz" after set "foo[. = 'bar']" "quux" = "foo=quux foo=baz"
test lns put "foo=bar foo=baz" after set "foo[. = 'baz']" "quux" = "foo=bar foo=quux"
test lns put "" after set "foo" "" = "foo"
