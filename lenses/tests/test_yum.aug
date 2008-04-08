module Test_yum =

  let yum_simple = "[sec1]
# comment
key=value
[sec2]
key1=value1
# comment
key2=value2
"

  let yum_conf = "[main]
cachedir=/var/cache/yum
keepcache=0
debuglevel=2
logfile=/var/log/yum.log
exactarch=1
obsoletes=1
gpgcheck=1
plugins=1
metadata_expire=1800

installonly_limit=100

# PUT YOUR REPOS HERE OR IN separate files named file.repo
# in /etc/yum.repos.d
"
(*
  test Yum.lns get yum_simple = 
    { "sec1" {} { "key" = "value" } }
    { "sec2" { "key1" = "value1" } {} { "key2" = "value2" } }

  test Yum.lns put yum_conf after
      rm "main" 
    = ""
*)  

  test Yum.lns put yum_simple after
      set "sec1/key1" "value1"
  = ? (* "[sec2]\nkey1=value1\n# comment\nkey2=value2\n" *)
  
(*
  test Yum.lns put yum_simple after
      rm "sec1" ;
      rm "sec2/key1"
  = "[sec2]\n# comment\nkey2=value2\n"
  
  test Yum.lns put yum_simple after
      rm "sec1" ;
      rm "sec2/key1" ;
      set "sec2/newkey" "newvalue"
  = "[sec2]\n# comment\nkey2=value2\nnewkey=newvalue\n"
  
  test Yum.lns put yum_simple after
      rm "sec1" ;
      set "sec2/key1" "newvalue"
  = "[sec2]\nkey1=newvalue\n# comment\nkey2=value2\n"
  
*)

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
