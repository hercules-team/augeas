(*
Module: Test_CPanel
  Provides unit tests and examples for the <CPanel> lens.
*)
module Test_CPanel =

(* Variable: config
     A sample cpanel.config file *)
let config = "#### NOTICE ####
# After manually editing any configuration settings in this file,
# please run '/usr/local/cpanel/whostmgr/bin/whostmgr2 --updatetweaksettings'
# to fully update your server's configuration.
 
skipantirelayd=1
ionice_optimizefs=6
account_login_access=owner_root
enginepl=cpanel.pl
stats_log=/usr/local/cpanel/logs/stats_log
cpaddons_notify_users=Allow users to choose
apache_port=0.0.0.0:80
allow_server_info_status_from=
system_diskusage_warn_percent=82.5500
maxemailsperhour
email_send_limits_max_defer_fail_percentage
default_archive-logs=1
SecurityPolicy::xml-api=1\n"

(* Test: CPanel.lns
     Get <config> *)
test CPanel.lns get config =
  { "#comment" = "### NOTICE ####" }
  { "#comment" = "After manually editing any configuration settings in this file," }
  { "#comment" = "please run '/usr/local/cpanel/whostmgr/bin/whostmgr2 --updatetweaksettings'" }
  { "#comment" = "to fully update your server's configuration." }
  {  }
  { "skipantirelayd" = "1" }
  { "ionice_optimizefs" = "6" }
  { "account_login_access" = "owner_root" }
  { "enginepl" = "cpanel.pl" }
  { "stats_log" = "/usr/local/cpanel/logs/stats_log" }
  { "cpaddons_notify_users" = "Allow users to choose" }
  { "apache_port" = "0.0.0.0:80" }
  { "allow_server_info_status_from" = "" }
  { "system_diskusage_warn_percent" = "82.5500" }
  { "maxemailsperhour" }
  { "email_send_limits_max_defer_fail_percentage" }
  { "default_archive-logs" = "1" }
  { "SecurityPolicy::xml-api" = "1" }
