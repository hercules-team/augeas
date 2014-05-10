module Test_Gshadow =

let conf = "root:x::
uucp:x::
sudo:x:suadmin1,suadmin2:coadmin1,coadmin2
"

test Gshadow.lns get conf =
  { "root"
    { "password" = "x" } }
  { "uucp"
    { "password" = "x" } }
  { "sudo"
    { "password" = "x" }
    { "admin" = "suadmin1" }
    { "admin" = "suadmin2" }
    { "member" = "coadmin1" }
    { "member" = "coadmin2" } }
