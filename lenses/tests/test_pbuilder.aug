

module Test_Pbuilder =

let conf = "BASETGZ=/var/cache/pbuilder/base.tgz
#EXTRAPACKAGES=gcc3.0-athlon-builder
export DEBIAN_BUILDARCH=athlon
BUILDPLACE=/var/cache/pbuilder/build/
MIRRORSITE=http://ftp.jp.debian.org/debian
"

test Pbuilder.lns get conf =
   { "BASETGZ"  = "/var/cache/pbuilder/base.tgz" }
   { "#comment" = "EXTRAPACKAGES=gcc3.0-athlon-builder" }
   { "DEBIAN_BUILDARCH" = "athlon"
       { "export" } }
   { "BUILDPLACE" = "/var/cache/pbuilder/build/" }
   { "MIRRORSITE" = "http://ftp.jp.debian.org/debian" }


