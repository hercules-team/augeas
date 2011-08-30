module Test_FAI_DiskConfig =


(* Test disk_config *)
let disk_config_test = "disk_config hda   preserve_always:6,7   disklabel:msdos  bootable:3
"

test FAI_DiskConfig.disk_config get disk_config_test =
  { "disk_config" = "hda"
    { "preserve_always" = "6,7" }
    { "disklabel" = "msdos" }
    { "bootable" = "3" }
  }

(* Test volume *)

let volume_test1 = "primary /boot     20-100        ext3            rw\n"
test FAI_DiskConfig.volume get volume_test1 =
    { "primary"
      { "mountpoint" = "/boot" }
      { "size" = "20-100" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
      }
    }

let volume_test2 = "primary swap      1000     swap       sw\n"
test FAI_DiskConfig.volume get volume_test2 =
    { "primary"
      { "mountpoint" = "swap" }
      { "size" = "1000" }
      { "filesystem" = "swap" }
      { "mount_options"
        { "1" = "sw" }
      }
    }

let volume_test3 = "primary /         12000      ext3           rw        createopts=\"-b 2048\"\n"
test FAI_DiskConfig.volume get volume_test3 =
    { "primary"
      { "mountpoint" = "/" }
      { "size" = "12000" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
      }
      { "fs_options"
        { "createopts" = "-b 2048" }
      }
    }

let volume_test4 = "logical /tmp      1000      ext3            rw,nosuid\n"
test FAI_DiskConfig.volume get volume_test4 =
    { "logical"
      { "mountpoint" = "/tmp" }
      { "size" = "1000" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
        { "2" = "nosuid" }
      }
    }

let volume_test5 = "logical /var      10%-      ext3               rw\n"
test FAI_DiskConfig.volume get volume_test5 =
    { "logical"
      { "mountpoint" = "/var" }
      { "size" = "10%-" }
      { "filesystem" = "ext3" }
      { "mount_options"
        { "1" = "rw" }
      }
    }

let volume_test6 = "logical /nobackup 0-        xfs                rw\n"
test FAI_DiskConfig.volume get volume_test6 =
    { "logical"
      { "mountpoint" = "/nobackup" }
      { "size" = "0-" }
      { "filesystem" = "xfs" }
      { "mount_options"
        { "1" = "rw" }
      }
    }

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
