(* Test for system lens *)
module Test_solaris_system =

  let conf = "*ident  \"@(#)system 1.18    97/06/27 SMI\" /* SVR4 1.5 */
*
* SYSTEM SPECIFICATION FILE
*

moddir: /kernel /usr/kernel /other/modules

rootfs:ufs
rootdev:/sbus@1,f8000000/esp@0,800000/sd@3,0:a

include: win
include: sys/shmsys

exclude: win
exclude: sys/shmsys

forceload: drv/foo
forceload: drv/ssd

set nautopush=32
set noexec_user_stack=1
set zfs:zfs_arc_max=12884901888
set test_module:debug = 0x13
set fcp:ssfcp_enable_auto_configuration=1
set scsi_options = 0x7F8
set moddebug & ~0x880
set moddebug | 0x40
"

  test Solaris_System.lns get conf =
    { "#comment" = "ident  \"@(#)system 1.18    97/06/27 SMI\" /* SVR4 1.5 */" }
    { }
    { "#comment" = "SYSTEM SPECIFICATION FILE" }
    { }
    { }
    { "moddir"
        { "1" = "/kernel" }
        { "2" = "/usr/kernel" }
        { "3" = "/other/modules" } }
    { }
    { "rootfs" = "ufs" }
    { "rootdev" = "/sbus@1,f8000000/esp@0,800000/sd@3,0:a" }
    { }
    { "include" = "win" }
    { "include" = "sys/shmsys" }
    { }
    { "exclude" = "win" }
    { "exclude" = "sys/shmsys" }
    { }
    { "forceload" = "drv/foo" }
    { "forceload" = "drv/ssd" }
    { }
    { "set"
        { "variable" = "nautopush" }
        { "operator" = "=" }
        { "value" = "32" } }
    { "set"
        { "variable" = "noexec_user_stack" }
        { "operator" = "=" }
        { "value" = "1" } }
    { "set"
        { "module" = "zfs" }
        { "variable" = "zfs_arc_max" }
        { "operator" = "=" }
        { "value" = "12884901888" } }
    { "set"
        { "module" = "test_module" }
        { "variable" = "debug" }
        { "operator" = "=" }
        { "value" = "0x13" } }
    { "set"
        { "module" = "fcp" }
        { "variable" = "ssfcp_enable_auto_configuration" }
        { "operator" = "=" }
        { "value" = "1" } }
    { "set"
        { "variable" = "scsi_options" }
        { "operator" = "=" }
        { "value" = "0x7F8" } }
    { "set"
        { "variable" = "moddebug" }
        { "operator" = "&" }
        { "value" = "~0x880" } }
    { "set"
        { "variable" = "moddebug" }
        { "operator" = "|" }
        { "value" = "0x40" } }

(* Check that moddir supports colons and spaces *)
  let moddir_colons = "moddir:/kernel:/usr/kernel:/other/modules
"

  test Solaris_System.lns get moddir_colons =
    { "moddir"
        { "1" = "/kernel" }
        { "2" = "/usr/kernel" }
        { "3" = "/other/modules" } }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
