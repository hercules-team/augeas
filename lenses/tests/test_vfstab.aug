module Test_vfstab =

  let simple = "/dev/dsk/c0t0d0s1 /dev/rdsk/c0t0d0s1 /test  ufs 1 yes ro\n"
  let simple_tree =
    { "1"
        { "spec" = "/dev/dsk/c0t0d0s1" }
        { "fsck" = "/dev/rdsk/c0t0d0s1" }
        { "file" = "/test" }
        { "vfstype" = "ufs" }
        { "passno" = "1" }
        { "atboot" = "yes" }
        { "opt" = "ro" } }
  test Vfstab.lns get simple = simple_tree

  let trailing_ws = "/dev/dsk/c0t0d0s1\t /dev/rdsk/c0t0d0s1\t /test\t ufs\t    1\t  yes\t  ro  \t  \n"
  test Vfstab.lns get trailing_ws = simple_tree

  (* Now test combinations where unneeded fields can be replaced by dashes and
     then should not appear in the tree. *)
  let gen_empty_field(fsck:string) (passno:string) (opt:string) =
    "/dev/dsk/c0t0d0s1\t " . fsck . "\t /test\t ufs\t " . passno . " yes " .  opt . "\t\n"

  (* Missing fsck *)
  let no_fsck = gen_empty_field "-" "1" "ro"
  test Vfstab.lns get no_fsck =
    { "1"
        { "spec" = "/dev/dsk/c0t0d0s1" }
        { "file" = "/test" }
        { "vfstype" = "ufs" }
        { "passno" = "1" }
        { "atboot" = "yes" }
        { "opt" = "ro" } }

  test Vfstab.lns put no_fsck after
    insa "fsck" "/1/spec" ;
    set "/1/fsck" "/dev/rdsk/c0t0d0s1" = gen_empty_field "/dev/rdsk/c0t0d0s1" "1" "ro"

  (* Missing passno *)
  let no_passno = gen_empty_field "/dev/rdsk/c0t0d0s1" "-" "ro"
  test Vfstab.lns get no_passno =
    { "1"
        { "spec" = "/dev/dsk/c0t0d0s1" }
        { "fsck" = "/dev/rdsk/c0t0d0s1" }
        { "file" = "/test" }
        { "vfstype" = "ufs" }
        { "atboot" = "yes" }
        { "opt" = "ro" } }

  test Vfstab.lns put no_passno after
    insa "passno" "/1/vfstype" ;
    set "/1/passno" "1" = gen_empty_field "/dev/rdsk/c0t0d0s1" "1" "ro"

  (* Missing opts *)
  let no_opts = gen_empty_field "/dev/rdsk/c0t0d0s1" "1" "-"
  test Vfstab.lns get no_opts =
    { "1"
        { "spec" = "/dev/dsk/c0t0d0s1" }
        { "fsck" = "/dev/rdsk/c0t0d0s1" }
        { "file" = "/test" }
        { "vfstype" = "ufs" }
        { "passno" = "1" }
        { "atboot" = "yes" } }

  test Vfstab.lns put no_opts after
    insa "opt" "/1/atboot" ;
    insa "opt" "/1/atboot" ;
    set "/1/opt[1]" "ro" ;
    set "/1/opt[2]" "fg"  = gen_empty_field "/dev/rdsk/c0t0d0s1" "1" "ro,fg"

  let multi_opts = "/dev/dsk/c0t0d0s1 /dev/rdsk/c0t0d0s1 /test  ufs 1 yes ro,nosuid,retry=5,fg\n"
  let multi_opts_tree =
    { "1"
        { "spec" = "/dev/dsk/c0t0d0s1" }
        { "fsck" = "/dev/rdsk/c0t0d0s1" }
        { "file" = "/test" }
        { "vfstype" = "ufs" }
        { "passno" = "1" }
        { "atboot" = "yes" }
        { "opt" = "ro" }
        { "opt" = "nosuid" }
        { "opt" = "retry"
            { "value" = "5" } }
        { "opt" = "fg" } }
  test Vfstab.lns get multi_opts = multi_opts_tree

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
