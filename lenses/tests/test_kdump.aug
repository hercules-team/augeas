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
path /var/crash  #comment after entry
core_collector makedumpfile -c
default poweroff
raw /dev/sda5
ext3 /dev/sda3
net my.server.com:/export/tmp
nfs my.server.com:/export/tmp
net user@my.server.com
ssh user@my.server.com
link_delay 60
kdump_pre /var/crash/scripts/kdump-pre.sh
kdump_post /var/crash/scripts/kdump-post.sh
#extra_bins /usr/bin/lftp /a/b/c
extra_bins /usr/bin/lftp  	   /a/b/c  # comment
disk_timeout 30
extra_modules gfs2 extra modules more
options babla 	 labl 	 kbak 	 	 df=dfg
options babla 	 labl 	 kbak 	 	 df=dfg
options babla 	 labl 	 kbak 	 	 df=dfg  # comment
sshkey /root/.ssh/kdump_id_rsa
force_rebuild 1
override_resettable 1
dracut_args --omit-drivers \"cfg80211 snd\" --add-drivers \"ext2 ext3\"
fence_kdump_args -p 7410 -f auto
fence_kdump_nodes 192.168.1.10 10.34.63.155
debug_mem_level 3
blacklist gfs2
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
    { "path" = "/var/crash"
      { "#comment" = "comment after entry" } }
    { "core_collector" = "makedumpfile -c" }
    { "default" = "poweroff" }
    { "raw" = "/dev/sda5" }
    { "ext3" = "/dev/sda3" }
    { "net" = "my.server.com:/export/tmp" }
    { "nfs" = "my.server.com:/export/tmp" }
    { "net" = "user@my.server.com" }
    { "ssh" = "user@my.server.com" }
    { "link_delay" = "60" }
    { "kdump_pre" = "/var/crash/scripts/kdump-pre.sh" }
    { "kdump_post" = "/var/crash/scripts/kdump-post.sh" }
    { "#comment" = "extra_bins /usr/bin/lftp /a/b/c" }
    { "extra_bins"
      { "1" = "/usr/bin/lftp" }
      { "2" = "/a/b/c" }
      { "#comment" = "comment" } }
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
        { "df" = "dfg" } }
      { "#comment" = "comment" } }
    { "sshkey" = "/root/.ssh/kdump_id_rsa" }
    { "force_rebuild" = "1" }
    { "override_resettable" = "1" }
    { "dracut_args" = "--omit-drivers \"cfg80211 snd\" --add-drivers \"ext2 ext3\"" }
    { "fence_kdump_args" = "-p 7410 -f auto" }
    { "fence_kdump_nodes"
      { "1" = "192.168.1.10" }
      { "2" = "10.34.63.155" } }
    { "debug_mem_level" = "3" }
    { "blacklist"
      { "1" = "gfs2" } }
