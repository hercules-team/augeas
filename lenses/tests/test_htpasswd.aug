(*
Module: Test_Htpasswd
  Provides unit tests and examples for the <Htpasswd> lens.
*)

module Test_Htpasswd =

let htpasswd = "foo-plain:bar
foo-crypt:78YuxG9nnfUCo
foo-md5:$apr1$NqCzyXmd$WLc/Wb35AkC.8tQQB3/Uw/
foo-sha1:{SHA}Ys23Ag/5IOWqZCw9QGaVDdHwH00=
"

test Htpasswd.lns get htpasswd =
  { "foo-plain" = "bar" }
  { "foo-crypt" = "78YuxG9nnfUCo" }
  { "foo-md5" = "$apr1$NqCzyXmd$WLc/Wb35AkC.8tQQB3/Uw/" }
  { "foo-sha1" = "{SHA}Ys23Ag/5IOWqZCw9QGaVDdHwH00=" }

