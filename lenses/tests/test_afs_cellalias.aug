(*
Module: Test_AFS_cellalias
  Provides unit tests and examples for the <AFS_cellalias> lens.
*)

module Test_AFS_cellalias =

(* Variable: conf
    A full configuration *)
let conf = "# Cell Aliases are meant to act like symlinks like '/afs/openafs.org -> oao'
# in root.afs, so sites relying on such a link for their cell can use dynroot.
# These aliases are set with 'fs newalias', or read from
# /usr/vice/etc/CellAlias
#
# Formating for /usr/vice/etc/CellAlias is in the form
# <target> <alias>
# an example would be
# fnal.gov/common/usr usr

fnal.gov fnal
fnal.gov/files fnal-files
"

(* Test: AFS_cellalias.lns
     Test the full <conf> *)
test AFS_cellalias.lns get conf = { "#comment" = "Cell Aliases are meant to act like symlinks like '/afs/openafs.org -> oao'" }
  { "#comment" = "in root.afs, so sites relying on such a link for their cell can use dynroot." }
  { "#comment" = "These aliases are set with 'fs newalias', or read from" }
  { "#comment" = "/usr/vice/etc/CellAlias" }
  {  }
  { "#comment" = "Formating for /usr/vice/etc/CellAlias is in the form" }
  { "#comment" = "<target> <alias>" }
  { "#comment" = "an example would be" }
  { "#comment" = "fnal.gov/common/usr usr" }
  {  }
  { "target" = "fnal.gov"
    { "linkname" = "fnal" }
  }
  { "target" = "fnal.gov/files"
    { "linkname" = "fnal-files" }
  }

