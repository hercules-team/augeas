(*
Module: Test_Kdump
  Provides unit tests and examples for the <Kdump> lens.
*)

module Test_Kdump =

   let conf = "# this is a comment
#another commented line

#comment after empty line
#
#comment after empty comment
path /var/crash
core_collector makedumpfile -c
default poweroff
raw /dev/sda5
ext3 /dev/sda3
net my.server.com:/export/tmp
net user@my.server.com
link_delay 60
kdump_post /var/crash/scripts/kdump-post.sh
#extra_bins /usr/bin/lftp /a/b/c
extra_bins /usr/bin/lftp  	   /a/b/c
disk_timeout 30
extra_modules gfs2 extra modules more
options babla 	 labl 	 kbak 	 	 df=dfg
options babla 	 labl 	 kbak 	 	 df=dfg
options babla 	 labl 	 kbak 	 	 df=dfg
"

  (* Test: Kdump.lns
     Check whole config file *)
  test Kdump.lns get conf =
    { "#comment" = "this is a comment" }
    { "#comment" = "another commented line" }
    {  }
    { "#comment" = "comment after empty line" }
    {  }
    { "#comment" = "comment after empty comment" }
    { "path" = "/var/crash" }
    { "core_collector" = "makedumpfile -c" }
    { "default" = "poweroff" }
    { "raw" = "/dev/sda5" }
    { "ext3" = "/dev/sda3" }
    { "net" = "my.server.com:/export/tmp" }
    { "net" = "user@my.server.com" }
    { "link_delay" = "60" }
    { "kdump_post" = "/var/crash/scripts/kdump-post.sh" }
    { "#comment" = "extra_bins /usr/bin/lftp /a/b/c" }
    { "extra_bins"
      { "1" = "/usr/bin/lftp" }
      { "2" = "/a/b/c" } }
    { "disk_timeout" = "30" }
    { "extra_modules"
      { "1" = "gfs2" }
      { "2" = "extra" }
      { "3" = "modules" }
      { "4" = "more" } }
    { "options"
      { "babla"
        { "labl" }
        { "kbak" }
        { "df" = "dfg" } } }
    { "options"
      { "babla"
        { "labl" }
        { "kbak" }
        { "df" = "dfg" } } }
    { "options"
      { "babla"
        { "labl" }
        { "kbak" }
        { "df" = "dfg" } } }
