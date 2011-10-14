module Test_schroot =

   let conf = "# Sample configuration

[sid]
type=plain
description=Debian unstable
description[fr_FR]=Debian instable
location=/srv/chroot/sid
priority=3
groups=sbuild
root-groups=root
aliases=unstable,default

[etch]
type=block-device
description=Debian testing
priority=2
#groups=sbuild-security
aliases=testing
device=/dev/hda_vg/etch_chroot
mount-options=-o atime
run-setup-scripts=true
run-exec-scripts=true

[sid-file]
type=file
description=Debian sid file-based chroot
priority=3
groups=sbuild
file=/srv/chroots/sid.tar.gz
run-setup-scripts=true
run-exec-scripts=true

[sid-snapshot]
type=lvm-snapshot
description=Debian unstable LVM snapshot
priority=3
groups=sbuild
root-groups=root
device=/dev/hda_vg/sid_chroot
mount-options=-o atime,sync,user_xattr
lvm-snapshot-options=--size 2G
run-setup-scripts=true
run-exec-scripts=true
"

   test Schroot.lns get conf =
  { "#comment" = "Sample configuration" }
  {  }
  { "sid"
    { "type" = "plain" }
    { "description" = "Debian unstable" }
    { "description" = "Debian instable"
      { "lang" = "fr_FR" }
    }
    { "location" = "/srv/chroot/sid" }
    { "priority" = "3" }
    { "groups" = "sbuild" }
    { "root-groups" = "root" }
    { "aliases" = "unstable,default" }
    {  }
  }
  { "etch"
    { "type" = "block-device" }
    { "description" = "Debian testing" }
    { "priority" = "2" }
    { "#comment" = "groups=sbuild-security" }
    { "aliases" = "testing" }
    { "device" = "/dev/hda_vg/etch_chroot" }
    { "mount-options" = "-o atime" }
    { "run-setup-scripts" = "true" }
    { "run-exec-scripts" = "true" }
    {  }
  }
  { "sid-file"
    { "type" = "file" }
    { "description" = "Debian sid file-based chroot" }
    { "priority" = "3" }
    { "groups" = "sbuild" }
    { "file" = "/srv/chroots/sid.tar.gz" }
    { "run-setup-scripts" = "true" }
    { "run-exec-scripts" = "true" }
    {  }
  }
  { "sid-snapshot"
    { "type" = "lvm-snapshot" }
    { "description" = "Debian unstable LVM snapshot" }
    { "priority" = "3" }
    { "groups" = "sbuild" }
    { "root-groups" = "root" }
    { "device" = "/dev/hda_vg/sid_chroot" }
    { "mount-options" = "-o atime,sync,user_xattr" }
    { "lvm-snapshot-options" = "--size 2G" }
    { "run-setup-scripts" = "true" }
    { "run-exec-scripts" = "true" }
  }
