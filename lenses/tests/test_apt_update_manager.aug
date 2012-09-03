(*
Module: Test_Apt_Update_Manager
  Provides unit tests and examples for the <Apt_Update_Manager> lens.
*)
module Test_Apt_Update_Manager =

(* Variable: meta_release *)
let meta_release = "# default location for the meta-release file

[METARELEASE]
URI = http://changelogs.ubuntu.com/meta-release
URI_LTS = http://changelogs.ubuntu.com/meta-release-lts
URI_UNSTABLE_POSTFIX = -development
URI_PROPOSED_POSTFIX = -proposed
"

(* Test: Apt_Update_Manager.lns *)
test Apt_Update_Manager.lns get meta_release =
  { "#comment" = "default location for the meta-release file" }
  {  }
  { "METARELEASE"
    { "URI" = "http://changelogs.ubuntu.com/meta-release" }
    { "URI_LTS" = "http://changelogs.ubuntu.com/meta-release-lts" }
    { "URI_UNSTABLE_POSTFIX" = "-development" }
    { "URI_PROPOSED_POSTFIX" = "-proposed" }
  }

(* Variable: release_upgrades *)
let release_upgrades = "# Default behavior for the release upgrader.

[DEFAULT]
Prompt=lts
"

(* Test: Apt_Update_Manager.lns *)
test Apt_Update_Manager.lns get release_upgrades =
  { "#comment" = "Default behavior for the release upgrader." }
  {  }
  { "DEFAULT"
    { "Prompt" = "lts" }
  }
