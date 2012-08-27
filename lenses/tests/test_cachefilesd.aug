module Test_Cachefilesd =

   let conf = "
# I am a comment
dir /var/cache/fscache
tAg mycache
brun 10%
bcull 7%
bstop 3%
frun 10%
fcull 7%
fstop 3%
nocull

secctx system_u:system_r:cachefiles_kernel_t:s0
"
   test Cachefilesd.lns get conf = 
    {  }
    { "#comment" = "I am a comment" }
    { "dir" = "/var/cache/fscache" }
    { "tAg" = "mycache" }
    { "brun" = "10%" }
    { "bcull" = "7%" }
    { "bstop" = "3%" }
    { "frun" = "10%" }
    { "fcull" = "7%" }
    { "fstop" = "3%" }
    { "nocull" }
    {  }
    { "secctx" = "system_u:system_r:cachefiles_kernel_t:s0" }
