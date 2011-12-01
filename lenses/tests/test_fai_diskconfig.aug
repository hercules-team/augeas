(*
Module: Test_FAI_DiskConfig
  Provides unit tests and examples for the <FAI_DiskConfig> lens.
*)

module Test_FAI_DiskConfig =


(* Test: FAI_DiskConfig.disk_config
   Test <FAI_DiskConfig.disk_config> *)
test FAI_DiskConfig.disk_config get
    "disk_config hda   preserve_always:6,7   disklabel:msdos  bootable:3\n" =

  { "disk_config" = "hda"
                    { "preserve_always"
                              { "1" = "6" }
                              { "2" = "7" } }
                    { "disklabel" = "msdos" }
                    { "bootable" = "3" } }

(* Test: FAI_DiskConfig.volume
   Test <FAI_DiskConfig.volume> *)
test FAI_DiskConfig.volume get
      "primary /boot     20-100        ext3            rw\n" =

    { "primary"
      { "mountpoint" = "/boot" }
      { "size" = "20-100" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" } } }

(* Test: FAI_DiskConfig.volume
   Testing <FAI_DiskConfig.volume> *)
test FAI_DiskConfig.volume get
      "primary swap      1000     swap       sw\n" =

    { "primary"
      { "mountpoint" = "swap" }
      { "size" = "1000" }
      { "filesystem" = "swap" }
      { "mount_options"
        { "1" = "sw" } } }

(* Test: FAI_DiskConfig.volume
   Testing <FAI_DiskConfig.volume> *)
test FAI_DiskConfig.volume get
      "primary /         12000      ext3           rw        createopts=\"-b 2048\"\n" =

    { "primary"
      { "mountpoint" = "/" }
      { "size" = "12000" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" } }
      { "fs_options"
        { "createopts" = "-b 2048" } } }

(* Test: FAI_DiskConfig.volume
   Testing <FAI_DiskConfig.volume> *)
test FAI_DiskConfig.volume get
      "logical /tmp      1000      ext3            rw,nosuid\n" =

    { "logical"
      { "mountpoint" = "/tmp" }
      { "size" = "1000" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
        { "2" = "nosuid" } } }

(* Test: FAI_DiskConfig.volume
   Testing <FAI_DiskConfig.volume> *)
test FAI_DiskConfig.volume get
      "logical /var      10%-      ext3               rw\n" =

    { "logical"
      { "mountpoint" = "/var" }
      { "size" = "10%-" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" } } }

(* Test: FAI_DiskConfig.volume
   Testing <FAI_DiskConfig.volume> *)
test FAI_DiskConfig.volume get
      "logical /nobackup 0-        xfs                rw\n" =

    { "logical"
      { "mountpoint" = "/nobackup" }
      { "size" = "0-" }
      { "filesystem" = "xfs" }
      { "mount_options"
        { "1" = "rw" } } }

(* Variable: simple_config
   A simple configuration file *)
let simple_config = "# A comment
disk_config disk2
raw-disk - 0 - -

disk_config lvm
vg my_pv        sda2
vg test		disk1.9
my_pv-_swap     swap    2048    swap    sw
my_pv-_root     /       2048    ext3 rw,errors=remount-ro

disk_config raid
raid1		/boot		disk1.1,disk2.1,disk3.1,disk4.1,disk5.1,disk6.1		ext3	rw
raid1		swap		disk1.2,disk2.2,disk3.2,disk4.2,disk5.2,disk6.2		swap	sw
raid5		/srv/data	disk1.11,disk2.11,disk3.11,disk4.11,disk5.11,disk6.11	ext3	ro		createopts=\"-m 0\"
raid0        -    disk2.2,sdc1,sde1:spare:missing  ext2       default

disk_config tmpfs
tmpfs                           /var/opt/hosting/tmp    500             defaults
"

(* Test: FAI_DiskConfig.lns
   Testing the full <FAI_DiskConfig.lns> on <simple_config> *)
test FAI_DiskConfig.lns get simple_config =
  { "#comment" = "A comment" }
  { "disk_config" = "disk2"
    { "raw-disk"
      { "mountpoint" = "-" }
      { "size" = "0" }
      { "filesystem" = "-" }
      { "mount_options"
        { "1" = "-" }
      }
    }
  }
  {  }
  { "disk_config" = "lvm"
    { "vg"
      { "name" = "my_pv" }
      { "disk" = "sda2" }
    }
    { "vg"
      { "name" = "test" }
      { "disk" = "disk1"
        { "partition" = "9" }
      }
    }
    { "lv"
      { "vg" = "my_pv" }
      { "name" = "_swap" }
      { "mountpoint" = "swap" }
      { "size" = "2048" }
      { "filesystem" = "swap" }
      { "mount_options"
        { "1" = "sw" }
      }
    }
    { "lv"
      { "vg" = "my_pv" }
      { "name" = "_root" }
      { "mountpoint" = "/" }
      { "size" = "2048" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
        { "2" = "errors"
          { "value" = "remount-ro" }
        }
      }
    }
  }
  {  }
  { "disk_config" = "raid"
    { "raid1"
      { "mountpoint" = "/boot" }
      { "disk" = "disk1"
        { "partition" = "1" }
      }
      { "disk" = "disk2"
        { "partition" = "1" }
      }
      { "disk" = "disk3"
        { "partition" = "1" }
      }
      { "disk" = "disk4"
        { "partition" = "1" }
      }
      { "disk" = "disk5"
        { "partition" = "1" }
      }
      { "disk" = "disk6"
        { "partition" = "1" }
      }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
      }
    }
    { "raid1"
      { "mountpoint" = "swap" }
      { "disk" = "disk1"
        { "partition" = "2" }
      }
      { "disk" = "disk2"
        { "partition" = "2" }
      }
      { "disk" = "disk3"
        { "partition" = "2" }
      }
      { "disk" = "disk4"
        { "partition" = "2" }
      }
      { "disk" = "disk5"
        { "partition" = "2" }
      }
      { "disk" = "disk6"
        { "partition" = "2" }
      }
      { "filesystem" = "swap" }
      { "mount_options"
        { "1" = "sw" }
      }
    }
    { "raid5"
      { "mountpoint" = "/srv/data" }
      { "disk" = "disk1"
        { "partition" = "11" }
      }
      { "disk" = "disk2"
        { "partition" = "11" }
      }
      { "disk" = "disk3"
        { "partition" = "11" }
      }
      { "disk" = "disk4"
        { "partition" = "11" }
      }
      { "disk" = "disk5"
        { "partition" = "11" }
      }
      { "disk" = "disk6"
        { "partition" = "11" }
      }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "ro" }
      }
      { "fs_options"
        { "createopts" = "-m 0" }
      }
    }
    { "raid0"
      { "mountpoint" = "-" }
      { "disk" = "disk2"
        { "partition" = "2" }
      }
      { "disk" = "sdc1" }
      { "disk" = "sde1"
        { "spare" }
        { "missing" }
      }
      { "filesystem" = "ext2" }
      { "mount_options"
        { "1" = "default" }
      }
    }
  }
  { }
  { "disk_config" = "tmpfs"
    { "tmpfs"
      { "mountpoint" = "/var/opt/hosting/tmp" }
      { "size" = "500" }
      { "mount_options"
        { "1" = "defaults" }
      }
    }
  }


(* Variable: config1
   Another full configuration *)
let config1 = "disk_config disk1 bootable:1 preserve_always:all always_format:5,6,7,8,9,10,11
primary  -         0   -     -
primary  -         0   -     -
logical  /         0   ext3  rw,relatime,errors=remount-ro   createopts=\"-c -j\"
logical  swap      0   swap  sw
logical  /var      0   ext3  rw,relatime                     createopts=\"-m 5 -j\"
logical  /tmp      0   ext3  rw                              createopts=\"-m 0 -j\"
logical  /usr      0   ext3  rw,relatime                     createopts=\"-j\"
logical  /home     0   ext3  rw,relatime,nosuid,nodev        createopts=\"-m 1 -j\"
logical  /wrk      0   ext3  rw,relatime,nosuid,nodev        createopts=\"-m 1 -j\"
logical  /transfer 0   vfat  rw
"

(* Test: FAI_DiskConfig.lns
   Testing <FAI_DiskConfig.lns> on <config1> *)
test FAI_DiskConfig.lns get config1 =
  { "disk_config" = "disk1"
    { "bootable" = "1" }
    { "preserve_always"
      { "1" = "all" }
    }
    { "always_format"
      { "1" = "5" }
      { "2" = "6" }
      { "3" = "7" }
      { "4" = "8" }
      { "5" = "9" }
      { "6" = "10" }
      { "7" = "11" }
    }
    { "primary"
      { "mountpoint" = "-" }
      { "size" = "0" }
      { "filesystem" = "-" }
      { "mount_options"
        { "1" = "-" }
      }
    }
    { "primary"
      { "mountpoint" = "-" }
      { "size" = "0" }
      { "filesystem" = "-" }
      { "mount_options"
        { "1" = "-" }
      }
    }
    { "logical"
      { "mountpoint" = "/" }
      { "size" = "0" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
        { "2" = "relatime" }
        { "3" = "errors"
          { "value" = "remount-ro" }
        }
      }
      { "fs_options"
        { "createopts" = "-c -j" }
      }
    }
    { "logical"
      { "mountpoint" = "swap" }
      { "size" = "0" }
      { "filesystem" = "swap" }
      { "mount_options"
        { "1" = "sw" }
      }
    }
    { "logical"
      { "mountpoint" = "/var" }
      { "size" = "0" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
        { "2" = "relatime" }
      }
      { "fs_options"
        { "createopts" = "-m 5 -j" }
      }
    }
    { "logical"
      { "mountpoint" = "/tmp" }
      { "size" = "0" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
      }
      { "fs_options"
        { "createopts" = "-m 0 -j" }
      }
    }
    { "logical"
      { "mountpoint" = "/usr" }
      { "size" = "0" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
        { "2" = "relatime" }
      }
      { "fs_options"
        { "createopts" = "-j" }
      }
    }
    { "logical"
      { "mountpoint" = "/home" }
      { "size" = "0" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
        { "2" = "relatime" }
        { "3" = "nosuid" }
        { "4" = "nodev" }
      }
      { "fs_options"
        { "createopts" = "-m 1 -j" }
      }
    }
    { "logical"
      { "mountpoint" = "/wrk" }
      { "size" = "0" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
        { "2" = "relatime" }
        { "3" = "nosuid" }
        { "4" = "nodev" }
      }
      { "fs_options"
        { "createopts" = "-m 1 -j" }
      }
    }
    { "logical"
      { "mountpoint" = "/transfer" }
      { "size" = "0" }
      { "filesystem" = "vfat" }
      { "mount_options"
        { "1" = "rw" }
      }
    }
  }


(* Variable: config2
   Another full configuration *)
let config2 = "disk_config /dev/sda
primary  -  250M  -  -
primary  -  20G   -  -
logical  -  8G    -  -
logical  -  4G    -  -
logical  -  5G    -  -

disk_config /dev/sdb sameas:/dev/sda

disk_config raid
raid1  /boot  sda1,sdb1  ext3  defaults
raid1  /      sda2,sdb2  ext3  defaults,errors=remount-ro
raid1  swap   sda5,sdb5  swap  defaults
raid1  /tmp   sda6,sdb6  ext3  defaults createopts=\"-m 1\"
raid1  /var   sda7,sdb7  ext3  defaults
"

(* Test: FAI_DiskConfig.lns
   Testing <FAI_DiskConfig.lns> on <config2> *)
test FAI_DiskConfig.lns get config2 =
  { "disk_config" = "/dev/sda"
    { "primary"
      { "mountpoint" = "-" }
      { "size" = "250M" }
      { "filesystem" = "-" }
      { "mount_options"
        { "1" = "-" }
      }
    }
    { "primary"
      { "mountpoint" = "-" }
      { "size" = "20G" }
      { "filesystem" = "-" }
      { "mount_options"
        { "1" = "-" }
      }
    }
    { "logical"
      { "mountpoint" = "-" }
      { "size" = "8G" }
      { "filesystem" = "-" }
      { "mount_options"
        { "1" = "-" }
      }
    }
    { "logical"
      { "mountpoint" = "-" }
      { "size" = "4G" }
      { "filesystem" = "-" }
      { "mount_options"
        { "1" = "-" }
      }
    }
    { "logical"
      { "mountpoint" = "-" }
      { "size" = "5G" }
      { "filesystem" = "-" }
      { "mount_options"
        { "1" = "-" }
      }
    }
  }
  {  }
  { "disk_config" = "/dev/sdb"
    { "sameas" = "/dev/sda" }
  }
  {  }
  { "disk_config" = "raid"
    { "raid1"
      { "mountpoint" = "/boot" }
      { "disk" = "sda1" }
      { "disk" = "sdb1" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "defaults" }
      }
    }
    { "raid1"
      { "mountpoint" = "/" }
      { "disk" = "sda2" }
      { "disk" = "sdb2" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "defaults" }
        { "2" = "errors"
          { "value" = "remount-ro" }
        }
      }
    }
    { "raid1"
      { "mountpoint" = "swap" }
      { "disk" = "sda5" }
      { "disk" = "sdb5" }
      { "filesystem" = "swap" }
      { "mount_options"
        { "1" = "defaults" }
      }
    }
    { "raid1"
      { "mountpoint" = "/tmp" }
      { "disk" = "sda6" }
      { "disk" = "sdb6" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "defaults" }
      }
      { "fs_options"
        { "createopts" = "-m 1" }
      }
    }
    { "raid1"
      { "mountpoint" = "/var" }
      { "disk" = "sda7" }
      { "disk" = "sdb7" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "defaults" }
      }
    }
  }


(* Variable: config3
   Another full configuration *)
let config3 = "disk_config /dev/sdb
primary  /      21750  ext3  defaults,errors=remount-ro
primary  /boot  250    ext3  defaults
logical  -      4000   -     -
logical  -      2000   -     -
logical  -      10-    -     -

disk_config cryptsetup randinit
swap  swap      /dev/sdb5  swap  defaults
tmp   /tmp      /dev/sdb6  ext2  defaults
luks  /local00  /dev/sdb7  ext3  defaults,errors=remount-ro  createopts=\"-m 0\"
"

(* Test: FAI_DiskConfig.lns
   Testing <FAI_DiskConfig.lns> on <config3> *)
test FAI_DiskConfig.lns get config3 =
  { "disk_config" = "/dev/sdb"
    { "primary"
      { "mountpoint" = "/" }
      { "size" = "21750" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "defaults" }
        { "2" = "errors"
          { "value" = "remount-ro" }
        }
      }
    }
    { "primary"
      { "mountpoint" = "/boot" }
      { "size" = "250" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "defaults" }
      }
    }
    { "logical"
      { "mountpoint" = "-" }
      { "size" = "4000" }
      { "filesystem" = "-" }
      { "mount_options"
        { "1" = "-" }
      }
    }
    { "logical"
      { "mountpoint" = "-" }
      { "size" = "2000" }
      { "filesystem" = "-" }
      { "mount_options"
        { "1" = "-" }
      }
    }
    { "logical"
      { "mountpoint" = "-" }
      { "size" = "10-" }
      { "filesystem" = "-" }
      { "mount_options"
        { "1" = "-" }
      }
    }
  }
  {  }
  { "disk_config" = "cryptsetup"
    { "randinit" }
    { "swap"
      { "mountpoint" = "swap" }
      { "device" = "/dev/sdb5" }
      { "filesystem" = "swap" }
      { "mount_options"
        { "1" = "defaults" }
      }
    }
    { "tmp"
      { "mountpoint" = "/tmp" }
      { "device" = "/dev/sdb6" }
      { "filesystem" = "ext2" }
      { "mount_options"
        { "1" = "defaults" }
      }
    }
    { "luks"
      { "mountpoint" = "/local00" }
      { "device" = "/dev/sdb7" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "defaults" }
        { "2" = "errors"
          { "value" = "remount-ro" }
        }
      }
      { "fs_options"
        { "createopts" = "-m 0" }
      }
    }
  }


(* Variable: with_spaces *)
let with_spaces = "disk_config disk2

raw-disk - 0 - -
"

(* Test: FAI_DiskConfig.lns
   Testing <FAI_DiskConfig.lns> with <with_spaces> *)
test FAI_DiskConfig.lns get with_spaces =
  { "disk_config" = "disk2"
    {  }
    { "raw-disk"
      { "mountpoint" = "-" }
      { "size" = "0" }
      { "filesystem" = "-" }
      { "mount_options"
        { "1" = "-" }
      }
    }
  }
