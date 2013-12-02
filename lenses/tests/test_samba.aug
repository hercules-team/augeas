module Test_samba =

   let conf = "#
# Sample configuration file for the Samba suite for Debian GNU/Linux.
#
#
# This is the main Samba configuration file. You should read the
# smb.conf(5) manual page in order to understand the options listed
# here. Samba has a huge number of configurable options most of which
# are not shown in this example
#

#======================= Global Settings =======================

[global]

## Browsing/Identification ###

# Change this to the workgroup/NT-domain name your Samba server will part of
   workgroup = WORKGROUP

# server string is the equivalent of the NT Description field
   server string = %h server (Samba, Ubuntu)

# Windows Internet Name Serving Support Section:
# WINS Support - Tells the NMBD component of Samba to enable its WINS Server
;   wins support = no

# Windows clients look for this share name as a source of downloadable
# printer drivers
[print$]
   comment = All Printers
   browseable = no
   path = /tmp
   printable = yes
   public = yes
   writable = no
   create mode = 0700
   printcap name = /etc/printcap
   print command = /usr/bin/lpr -P%p -r %s
   printing = cups
"

   test Samba.lns get conf =
      {}
      { "#comment" = "Sample configuration file for the Samba suite for Debian GNU/Linux." }
      {}
      {}
      { "#comment" = "This is the main Samba configuration file. You should read the" }
      { "#comment" = "smb.conf(5) manual page in order to understand the options listed" }
      { "#comment" = "here. Samba has a huge number of configurable options most of which" }
      { "#comment" = "are not shown in this example" }
      {}
      {}
      { "#comment" = "======================= Global Settings =======================" }
      {}
      { "target" = "global"
         {}
         { "#comment" = "# Browsing/Identification ###" }
         {}
         { "#comment" = "Change this to the workgroup/NT-domain name your Samba server will part of" }
         { "workgroup"  = "WORKGROUP" }
         {}
         { "#comment" = "server string is the equivalent of the NT Description field" }
         { "server string" = "%h server (Samba, Ubuntu)" }
         {}
         { "#comment" = "Windows Internet Name Serving Support Section:" }
         { "#comment" = "WINS Support - Tells the NMBD component of Samba to enable its WINS Server" }
         { "#comment" = "wins support = no" }
         {}
         { "#comment" = "Windows clients look for this share name as a source of downloadable" }
         { "#comment" = "printer drivers" } }
      { "target" = "print$"
         { "comment" = "All Printers" }
         { "browseable" = "no" }
         { "path" = "/tmp" }
         { "printable" = "yes" }
         { "public" = "yes" }
         { "writable" = "no" }
         { "create mode" = "0700" }
         { "printcap name" = "/etc/printcap" }
         { "print command" = "/usr/bin/lpr -P%p -r %s" }
         { "printing" = "cups" } }

    test Samba.lns get "[test]\ncrazy:entry = foo\n" =
         { "target" = "test"
            {"crazy:entry" = "foo"}}

    (* Test complex idmap commands with asterisk in key name, ticket #354 *)
    test Samba.lns get "[test]
  idmap backend = tdb
  idmap uid = 1000000-1999999
  idmap gid = 1000000-1999999

  idmap config CORP : backend  = ad
  idmap config * : range = 1000-999999\n" =
      { "target" = "test"
        { "idmap backend" = "tdb" }
        { "idmap uid" = "1000000-1999999" }
        { "idmap gid" = "1000000-1999999" }
        {  }
        { "idmap config CORP : backend" = "ad" }
        { "idmap config * : range" = "1000-999999" } }
