module Test_yum =

  let yum_simple = "[sec1]
# comment
key=value
[sec-two]
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

  let cont = "[main]\nbaseurl=url1\n   url2 , url3\n   \n"

  test Yum.lns get yum_simple =
    { "sec1" {} { "key" = "value" } }
    { "sec-two" { "key1" = "value1" } {} { "key2" = "value2" } }

  test Yum.lns put yum_conf after
      rm "main"
    = ""

  test Yum.lns put yum_simple after
      set "sec1/key" "othervalue"
    = "[sec1]\n# comment\nkey=othervalue\n[sec-two]\nkey1=value1\n# comment\nkey2=value2\n"

  test Yum.lns put yum_simple after
      rm "sec1" ;
      rm "sec-two/key1"
  = "[sec-two]\n# comment\nkey2=value2\n"

  test Yum.lns put yum_simple after
      rm "sec1" ;
      rm "sec-two/key1" ;
      set "sec-two/newkey" "newvalue"
  = "[sec-two]\n# comment\nkey2=value2\nnewkey=newvalue\n"

  test Yum.lns put yum_simple after
      rm "sec1" ;
      set "sec-two/key1" "newvalue"
   = "[sec-two]\nkey1=newvalue\n# comment\nkey2=value2\n"

  test Yum.lns get cont =
    { "main"
        { "baseurl" = "url1" }
        { "baseurl" = "url2" }
        { "baseurl" = "url3" }
        {}
    }

  test Yum.lns put cont after
      set "main/gpgcheck" "1"
  =
    cont . "gpgcheck=1\n"

  (* We are actually stricter than yum in checking syntax. The yum.conf *)
  (* man page mentions that it is illegal to have multiple baseurl keys *)
  (* in the same section, but yum will just carry on, usually with      *)
  (* results that surpise the unsuspecting user                         *)
  test Yum.lns get "[repo]\nbaseurl=url1\nbaseurl=url2\n" = *

  (* This checks that we take the right branch in the section lens.     *)
  test Yum.section get "[repo]\nname=A name\nbaseurl=url1\n" =
    { "repo"
        { "name" = "A name" }
        { "baseurl" = "url1" } }

  (* Handle continuation lines for gpgkey; bug #132 *)
  test Yum.lns get "[main]\ngpgkey=key1\n  key2\n" =
    { "main"
        { "gpgkey" = "key1" }
        { "gpgkey" = "key2" } }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
