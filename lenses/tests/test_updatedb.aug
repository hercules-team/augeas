(*
Module: Test_UpdateDB
  Provides unit tests and examples for the <UpdateDB> lens.
*)
module Test_UpdateDB =

(* Test: UpdateDB.lns
     Simple get test *)
test UpdateDB.lns get "# A comment
PRUNEPATHS=\"/tmp /var/spool /media /home/.ecryptfs\"
PRUNEFS= \"NFS nfs nfs4 rpc_pipefs\"
PRUNE_BIND_MOUNTS = \"yes\"\n" =
  { "#comment" = "A comment" }
  { "PRUNEPATHS"
    { "entry" = "/tmp" }
    { "entry" = "/var/spool" }
    { "entry" = "/media" }
    { "entry" = "/home/.ecryptfs" }
  }
  { "PRUNEFS"
    { "entry" = "NFS" }
    { "entry" = "nfs" }
    { "entry" = "nfs4" }
    { "entry" = "rpc_pipefs" }
  }
  { "PRUNE_BIND_MOUNTS" = "yes" }

(* Test: UpdateDB.lns
     Adding to a list *)
test UpdateDB.lns put "PRUNEFS=\"NFS nfs nfs4 rpc_pipefs\"\n"
  after set "/PRUNEFS/entry[last()+1]" "ecryptfs" =
"PRUNEFS=\"NFS nfs nfs4 rpc_pipefs ecryptfs\"\n"
