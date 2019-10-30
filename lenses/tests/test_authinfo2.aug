module Test_authinfo2 =

let conf = "# Comment
[s3]
storage-url: s3://
backend-login: joe
backend-password: notquitesecret

[fs1]
storage-url: s3://joes-first-bucket
fs-passphrase: neitheristhis

[fs2]

storage-url: s3://joes-second-bucket
fs-passphrase: swordfish
[fs3]
storage-url: s3://joes-second-bucket/with-prefix
backend-login: bill
backend-password: bi23ll
fs-passphrase: ll23bi
"


test Authinfo2.lns get conf = 
  { "#comment" = "Comment" }
  { "s3"
     { "storage-url" = "s3://" }
     { "backend-login" = "joe" }
     { "backend-password" = "notquitesecret" }
     {} }
  { "fs1"
     { "storage-url" = "s3://joes-first-bucket" }
     { "fs-passphrase"  = "neitheristhis" }
     {} }
  { "fs2"
     {}
     { "storage-url" = "s3://joes-second-bucket" }
     { "fs-passphrase" = "swordfish" } }
  { "fs3"
     { "storage-url" = "s3://joes-second-bucket/with-prefix" }
     { "backend-login" = "bill" }
     { "backend-password" = "bi23ll" }
     { "fs-passphrase" = "ll23bi" } }
