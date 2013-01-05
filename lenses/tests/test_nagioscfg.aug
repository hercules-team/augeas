(*
Module: Test_NagiosCfg
  Provides unit tests and examples for the <NagiosCfg> lens.
*)

module Test_NagiosCfg =
    let conf="
# LOG FILE
log_file=/var/log/nagios3/nagios.log

# OBJECT CONFIGURATION FILE(S)
cfg_file=/etc/nagios3/objects/check_commands.cfg
cfg_file=/etc/nagios3/objects/contact_groups.cfg
cfg_file=/etc/nagios3/objects/contacts.cfg
cfg_file=/etc/nagios3/objects/hostgroups.cfg
cfg_file=/etc/nagios3/objects/hosts.cfg
cfg_file=/etc/nagios3/objects/services.cfg

# NAGIOS USER
nagios_user=nagios

# NAGIOS GROUP
nagios_group=nagios

# DATE FORMAT
date_format=iso8601

# ILLEGAL OBJECT NAME CHARS
illegal_object_name_chars=`~!$%^&*|'\"<>?,()'=

# ILLEGAL MACRO OUTPUT CHARS
illegal_macro_output_chars=`~$&|'\"<>

# MISC DIRECTIVES
p1_file=/usr/lib/nagios3/p1.pl
event_broker_options=-1
use_large_installation_tweaks=1
broker_module=/usr/lib/nagios3/libNagiosCluster-1.0.so.4.0.0
broker_module=/usr/sbin/ndomod.o config_file=/etc/nagios3/ndomod.cfg
"

    test NagiosCfg.lns get conf =
        {}
        { "#comment"     = "LOG FILE" }
        { "log_file"    = "/var/log/nagios3/nagios.log" }
        {}
        { "#comment"     = "OBJECT CONFIGURATION FILE(S)" }
        { "cfg_file"    = "/etc/nagios3/objects/check_commands.cfg" }
        { "cfg_file"    = "/etc/nagios3/objects/contact_groups.cfg" }
        { "cfg_file"    = "/etc/nagios3/objects/contacts.cfg" }
        { "cfg_file"    = "/etc/nagios3/objects/hostgroups.cfg" }
        { "cfg_file"    = "/etc/nagios3/objects/hosts.cfg" }
        { "cfg_file"    = "/etc/nagios3/objects/services.cfg" }
        {}
        { "#comment"     = "NAGIOS USER" }
        { "nagios_user" = "nagios" }
        {}
        { "#comment"     = "NAGIOS GROUP" }
        { "nagios_group"= "nagios" }
        {}
        { "#comment"     = "DATE FORMAT" }
        { "date_format" = "iso8601" }
        {}
        { "#comment"     = "ILLEGAL OBJECT NAME CHARS" }
        { "illegal_object_name_chars"   = "`~!$%^&*|'\"<>?,()'=" }
        {}
        { "#comment"     = "ILLEGAL MACRO OUTPUT CHARS" }
        { "illegal_macro_output_chars"  = "`~$&|'\"<>" }
        {}
        { "#comment"     = "MISC DIRECTIVES" }
        { "p1_file"     = "/usr/lib/nagios3/p1.pl" }
        { "event_broker_options"    = "-1" }
        { "use_large_installation_tweaks"   = "1" }
        { "broker_module"   = "/usr/lib/nagios3/libNagiosCluster-1.0.so.4.0.0" }
        { "broker_module"   = "/usr/sbin/ndomod.o"
           { "config_file" = "/etc/nagios3/ndomod.cfg" } }


(* Spaces are fine in values *)
let space_in = "nagios_check_command=/usr/lib/nagios/plugins/check_nagios /var/cache/nagios3/status.dat 5 '/usr/sbin/nagios3'\n"

test NagiosCfg.lns get space_in =
  { "nagios_check_command" = "/usr/lib/nagios/plugins/check_nagios /var/cache/nagios3/status.dat 5 '/usr/sbin/nagios3'" }

test NagiosCfg.lns get "$USER1$=/usr/local/libexec/nagios\n" =
  { "$USER1$" = "/usr/local/libexec/nagios" }

test NagiosCfg.lns get "$USER3$=somepassword\n" =
  { "$USER3$" = "somepassword" }
