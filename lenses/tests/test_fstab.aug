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

  let no_passno = "LABEL=/boot\t /boot\t ext3\t    defaults        1  \t\n"

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

  
  let multi_opts = "devpts\t /dev/pts\t devpts  gid=5,mode=620  0 0\n"

  let multi_opts_tree = 
    { "1"
        { "spec" = "devpts" }
        { "file" = "/dev/pts" }
        { "vfstype" = "devpts" }
        { "opt" = "gid=5" }
        { "opt" = "mode=620" }
        { "dump" = "0" }
        { "passno" = "0" } }
  
  test Fstab.lns get simple = simple_tree

  test Fstab.lns get trailing_ws = simple_tree

  test Fstab.lns get no_passno = no_passno_tree

  test Fstab.lns put no_passno after set "1/passno" "1" = 
    "LABEL=/boot\t /boot\t ext3\t    defaults\t1 1  \t\n"

  test Fstab.lns get no_dump = no_dump_tree

  test Fstab.lns get multi_opts = multi_opts_tree

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
