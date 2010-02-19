module Test_NagiosCfg =

	let s1 = "log_file=/var/log/nagios3/nagios.log\n"
	test NagiosCfg.lns get s1 =
		{ "log_file" = "/var/log/nagios3/nagios.log" }

	let s2 = "debug_level=0



# DEBUG VERBOSITY
# This option determines how verbose the debug log out will be.
# Values: 0 = Brief output
#         1 = More detailed
#         2 = Very detailed

debug_verbosity=1



# DEBUG FILE
# This option determines where Nagios should write debugging information.

debug_file=/var/lib/nagios3/nagios.debug



# MAX DEBUG FILE SIZE
# This option determines the maximum size (in bytes) of the debug file.  If
# the file grows larger than this size, it will be renamed with a .old
# extension.  If a file already exists with a .old extension it will
# automatically be deleted.  This helps ensure your disk space usage doesn't
# get out of control when debugging Nagios.

max_debug_file_size=1000000


cfg_dir=/etc/nagios3/dcsit_production
cfg_dir=/etc/nagios3/dcdb_test
cfg_dir=/etc/nagios3/dcdb_production
"
	test NagiosCfg.lns get s2 =
  { "debug_level" = "0" }
  {  }
  {  }
  {  }
  { "#comment" = "DEBUG VERBOSITY" }
  { "#comment" = "This option determines how verbose the debug log out will be." }
  { "#comment" = "Values: 0 = Brief output" }
  { "#comment" = "1 = More detailed" }
  { "#comment" = "2 = Very detailed" }
  {  }
  { "debug_verbosity" = "1" }
  {  }
  {  }
  {  }
  { "#comment" = "DEBUG FILE" }
  { "#comment" = "This option determines where Nagios should write debugging information." }
  {  }
  { "debug_file" = "/var/lib/nagios3/nagios.debug" }
  {  }
  {  }
  {  }
  { "#comment" = "MAX DEBUG FILE SIZE" }
  { "#comment" = "This option determines the maximum size (in bytes) of the debug file.  If" }
  { "#comment" = "the file grows larger than this size, it will be renamed with a .old" }
  { "#comment" = "extension.  If a file already exists with a .old extension it will" }
  { "#comment" = "automatically be deleted.  This helps ensure your disk space usage doesn't" }
  { "#comment" = "get out of control when debugging Nagios." }
  {  }
  { "max_debug_file_size" = "1000000" }
  {  }
  {  }
  { "cfg_dir" = "/etc/nagios3/dcsit_production" }
  { "cfg_dir" = "/etc/nagios3/dcdb_test" }
  { "cfg_dir" = "/etc/nagios3/dcdb_production" }
