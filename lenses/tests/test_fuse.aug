(*
Module: Test_Fuse
  Provides unit tests and examples for the <Fuse> lens.
*)

module Test_Fuse =

(* Variable: conf *)
let conf = "# Set the maximum number of FUSE mounts allowed to non-root users.
mount_max = 1000

# Allow non-root users to specify the 'allow_other' or 'allow_root'
user_allow_other
"

(* Test: Fuse.lns *)
test Fuse.lns get conf =
   { "#comment" = "Set the maximum number of FUSE mounts allowed to non-root users." }
   { "mount_max" = "1000" }
   {  }
   { "#comment" = "Allow non-root users to specify the 'allow_other' or 'allow_root'" }
   { "user_allow_other" }
