module Test_fstab =

  let simple = "/dev/vg00/lv00\t /\t ext3\t    defaults        1 1\n"

  let simple_tree =
    { "1"
        { "spec" = "/dev/vg00/lv00" }
        { "file" = "/" }
        { "vfstype" = "ext3" }
        { "opt" = "defaults" }
        { "dump" = "1" }
        { "passno" = "1" } }

  let trailing_ws = "/dev/vg00/lv00\t /\t ext3\t    defaults        1 1  \t\n"

  let gen_no_passno(passno:string) =
    "LABEL=/boot\t /boot\t ext3\t    defaults        1" . passno . "  \t\n"
  let no_passno = gen_no_passno ""

  let no_passno_tree =
    { "1"
        { "spec" = "LABEL=/boot" }
        { "file" = "/boot" }
        { "vfstype" = "ext3" }
        { "opt" = "defaults" }
        { "dump" = "1" } }

  let no_dump = "/dev/vg00/lv00\t /\t ext3\t    defaults\n"

  let no_dump_tree =
    { "1"
        { "spec" = "/dev/vg00/lv00" }
        { "file" = "/" }
        { "vfstype" = "ext3" }
        { "opt" = "defaults" } }

  let no_opts = "/dev/vg00/lv00\t /\t ext3\n"

  let no_opts_tree =
    { "1"
        { "spec" = "/dev/vg00/lv00" }
        { "file" = "/" }
        { "vfstype" = "ext3" } }

  let multi_opts = "devpts\t /dev/pts\t devpts  gid=5,mode=620,fscontext=system_u:object_r:removable_t  0 0\n"

  let multi_opts_tree =
    { "1"
        { "spec" = "devpts" }
        { "file" = "/dev/pts" }
        { "vfstype" = "devpts" }
        { "opt" = "gid"
            { "value" = "5" } }
        { "opt" = "mode"
            { "value" = "620" } }
        { "opt" = "fscontext"
            { "value" = "system_u:object_r:removable_t" } }
        { "dump" = "0" }
        { "passno" = "0" } }

  test Fstab.lns get simple = simple_tree

  test Fstab.lns get trailing_ws = simple_tree

  test Fstab.lns get no_passno = no_passno_tree

  test Fstab.lns put no_passno after set "/1/passno" "1" = gen_no_passno " 1"

  test Fstab.lns get no_dump = no_dump_tree

  test Fstab.lns get no_opts = no_opts_tree

  test Fstab.lns get multi_opts = multi_opts_tree

  test Fstab.lns get "/dev/hdc        /media/cdrom0   udf,iso9660 user,noauto\t0\t0\n" =
    { "1"
        { "spec" = "/dev/hdc" }
        { "file" = "/media/cdrom0" }
        { "vfstype" = "udf" }
        { "vfstype" = "iso9660" }
        { "opt" = "user" }
        { "opt" = "noauto" }
        { "dump" = "0" }
        { "passno" = "0" } }

  (* Allow # in the spec *)
  test Fstab.lns get "sshfs#jon@10.0.0.2:/home    /media/server    fuse    uid=1000,gid=100,port=1022 0 0\n" =
  { "1"
    { "spec" = "sshfs#jon@10.0.0.2:/home" }
    { "file" = "/media/server" }
    { "vfstype" = "fuse" }
    { "opt" = "uid"
         { "value" = "1000" } }
    { "opt" = "gid"
         { "value" = "100" } }
    { "opt" = "port"
         { "value" = "1022" } }
    { "dump" = "0" }
    { "passno" = "0" } }

  (* Bug #191 *)
  test Fstab.lns get "tmpfs /dev/shm tmpfs rw,rootcontext=\"system_u:object_r:tmpfs_t:s0\" 0 0\n" =
  { "1"
    { "spec" = "tmpfs" }
    { "file" = "/dev/shm" }
    { "vfstype" = "tmpfs" }
    { "opt" = "rw" }
    { "opt" = "rootcontext"
      { "value" = "\"system_u:object_r:tmpfs_t:s0\"" } }
    { "dump" = "0" }
    { "passno" = "0" } }

  (* BZ https://bugzilla.redhat.com/show_bug.cgi?id=751342
   * Mounting multiple cgroups together results in path with ','
   *)
  test Fstab.lns get "spec /path/file1,file2 vfs opts 0 0\n" =
  { "1"
    { "spec" = "spec" }
    { "file" = "/path/file1,file2" }
    { "vfstype" = "vfs" }
    { "opt" = "opts" }
    { "dump" = "0" }
    { "passno" = "0" } }

  (* Parse when empty option value given, only equals sign *)
  test Fstab.lns get "//host.example.org/a_share /mnt cifs defaults,ro,password= 0 0\n" =
  { "1"
    { "spec" = "//host.example.org/a_share" }
    { "file" = "/mnt" }
    { "vfstype" = "cifs" }
    { "opt" = "defaults" }
    { "opt" = "ro" }
    { "opt" = "password"
      { "value" }
    }
    { "dump" = "0" }
    { "passno" = "0" }
  }

  (* Allow end of line comments *)
  test Fstab.lns get "UUID=0314be77-bb1e-47d4-b2a2-e69ae5bc954f	/	ext4	rw,errors=remount-ro	0	1	# device at install: /dev/sda3\n" =
  { "1"
    { "spec" = "UUID=0314be77-bb1e-47d4-b2a2-e69ae5bc954f" }
    { "file" = "/" }
    { "vfstype" = "ext4" }
    { "opt" = "rw" }
    { "opt" = "errors"
      { "value" = "remount-ro" }
    }
    { "dump" = "0" }
    { "passno" = "1" }
    { "#comment" = "device at install: /dev/sda3" }
  }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
