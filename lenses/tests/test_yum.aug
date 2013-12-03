(*
Module: Test_Yum
  Provides unit tests and examples for the <Yum> lens.
*)

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

  let yum_repo1 = "[fedora]
name=Fedora $releasever - $basearch
failovermethod=priority
#baseurl=http://download.fedora.redhat.com/pub/fedora/linux/releases/$releasever/Everything/$basearch/os/
mirrorlist=http://mirrors.fedoraproject.org/mirrorlist?repo=fedora-$releasever&arch=$basearch
enabled=1
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora file:///etc/pki/rpm-gpg/RPM-GPG-KEY

[fedora-debuginfo]
name=Fedora $releasever - $basearch - Debug
failovermethod=priority
#baseurl=http://download.fedora.redhat.com/pub/fedora/linux/releases/$releasever/Everything/$basearch/debug/
mirrorlist=http://mirrors.fedoraproject.org/mirrorlist?repo=fedora-debug-$releasever&arch=$basearch
enabled=0
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora file:///etc/pki/rpm-gpg/RPM-GPG-KEY

[fedora-source]
name=Fedora $releasever - Source
failovermethod=priority
#baseurl=http://download.fedora.redhat.com/pub/fedora/linux/releases/$releasever/Everything/source/SRPMS/
mirrorlist=http://mirrors.fedoraproject.org/mirrorlist?repo=fedora-source-$releasever&arch=$basearch
enabled=0
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora file:///etc/pki/rpm-gpg/RPM-GPG-KEY
"
  let yum_repo2 = "[remi]
name=Les RPM de remi pour FC$releasever - $basearch
baseurl=http://remi.collet.free.fr/rpms/fc$releasever.$basearch/
    http://iut-info.ens.univ-reims.fr/remirpms/fc$releasever.$basearch/
enabled=0
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-remi

[remi-test]
name=Les RPM de remi en test pour FC$releasever - $basearch
baseurl=http://remi.collet.free.fr/rpms/test-fc$releasever.$basearch/
    http://iut-info.ens.univ-reims.fr/remirpms/test-fc$releasever.$basearch/
enabled=0
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-remi
"

  let cont = "[main]\nbaseurl=url1\n   url2 , url3\n   \n"

  test Yum.lns get yum_simple =
  { "sec1"
    { "#comment" = "comment" }
    { "key" = "value" }
  }
  { "sec-two"
    { "key1" = "value1" }
    { "#comment" = "comment" }
    { "key2" = "value2" }
  }

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
  test Yum.record get "[repo]\nname=A name\nbaseurl=url1\n" =
    { "repo"
        { "name" = "A name" }
        { "baseurl" = "url1" } }

  (* Handle continuation lines for gpgkey; bug #132 *)
  test Yum.lns get "[main]\ngpgkey=key1\n  key2\n" =
    { "main"
        { "gpgkey" = "key1" }
        { "gpgkey" = "key2" } }

  test Yum.lns get yum_repo1 =
  { "fedora"
    { "name" = "Fedora $releasever - $basearch" }
    { "failovermethod" = "priority" }
    { "#comment" = "baseurl=http://download.fedora.redhat.com/pub/fedora/linux/releases/$releasever/Everything/$basearch/os/" }
    { "mirrorlist" = "http://mirrors.fedoraproject.org/mirrorlist?repo=fedora-$releasever&arch=$basearch" }
    { "enabled" = "1" }
    { "gpgcheck" = "1" }
    { "gpgkey" = "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora" }
    { "gpgkey" = "file:///etc/pki/rpm-gpg/RPM-GPG-KEY" }
    {  }
  }
  { "fedora-debuginfo"
    { "name" = "Fedora $releasever - $basearch - Debug" }
    { "failovermethod" = "priority" }
    { "#comment" = "baseurl=http://download.fedora.redhat.com/pub/fedora/linux/releases/$releasever/Everything/$basearch/debug/" }
    { "mirrorlist" = "http://mirrors.fedoraproject.org/mirrorlist?repo=fedora-debug-$releasever&arch=$basearch" }
    { "enabled" = "0" }
    { "gpgcheck" = "1" }
    { "gpgkey" = "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora" }
    { "gpgkey" = "file:///etc/pki/rpm-gpg/RPM-GPG-KEY" }
    {  }
  }
  { "fedora-source"
    { "name" = "Fedora $releasever - Source" }
    { "failovermethod" = "priority" }
    { "#comment" = "baseurl=http://download.fedora.redhat.com/pub/fedora/linux/releases/$releasever/Everything/source/SRPMS/" }
    { "mirrorlist" = "http://mirrors.fedoraproject.org/mirrorlist?repo=fedora-source-$releasever&arch=$basearch" }
    { "enabled" = "0" }
    { "gpgcheck" = "1" }
    { "gpgkey" = "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora" }
    { "gpgkey" = "file:///etc/pki/rpm-gpg/RPM-GPG-KEY" }
  }


  test Yum.lns get yum_repo2 =
  { "remi"
    { "name" = "Les RPM de remi pour FC$releasever - $basearch" }
    { "baseurl" = "http://remi.collet.free.fr/rpms/fc$releasever.$basearch/" }
    { "baseurl" = "http://iut-info.ens.univ-reims.fr/remirpms/fc$releasever.$basearch/" }
    { "enabled" = "0" }
    { "gpgcheck" = "1" }
    { "gpgkey" = "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-remi" }
    {  }
  }
  { "remi-test"
    { "name" = "Les RPM de remi en test pour FC$releasever - $basearch" }
    { "baseurl" = "http://remi.collet.free.fr/rpms/test-fc$releasever.$basearch/" }
    { "baseurl" = "http://iut-info.ens.univ-reims.fr/remirpms/test-fc$releasever.$basearch/" }
    { "enabled" = "0" }
    { "gpgcheck" = "1" }
    { "gpgkey" = "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-remi" }
  }

  (* Test: Yum.lns
      Check that we can parse an empty line, to fix test-save *)
  test Yum.lns get "\n" = { }

  (* Test: Yum.lns
       Issue #45: allow spaces around equals sign *)
  test Yum.lns get "[rpmforge]
name = RHEL $releasever - RPMforge.net - dag
baseurl = http://apt.sw.be/redhat/el6/en/$basearch/rpmforge\n" =
    { "rpmforge"
      { "name" = "RHEL $releasever - RPMforge.net - dag" }
      { "baseurl" = "http://apt.sw.be/redhat/el6/en/$basearch/rpmforge" }
    }

  (* Test: Yum.lns
       Issue #275: parse excludes as a list *)
  test Yum.lns get "[epel]
name=Extra Packages for Enterprise Linux 6 - $basearch
exclude=ocs* clamav*
" =
    { "epel"
      { "name" = "Extra Packages for Enterprise Linux 6 - $basearch" }
      { "exclude" = "ocs*" }
      { "exclude" = "clamav*" } }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
