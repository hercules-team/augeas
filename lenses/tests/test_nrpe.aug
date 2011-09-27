module Test_nrpe =

  let command = "command[foo]=bar\n"

  test Nrpe.command get command =
    { "command"
      { "foo" = "bar" }
    }


  let item = "nrpe_user=nagios\n"

  test Nrpe.item get item =
    { "nrpe_user" = "nagios" }


  let include = "include=/path/to/file.cfg\n"

  test Nrpe.include get include =
    { "include"
      { "file" = "/path/to/file.cfg" }
    }


  let comment = "# a comment\n"

  test Nrpe.comment get comment =
    { "#comment" = "a comment" }


  let empty = "# \n"

  test Nrpe.empty get empty =
    { }


  let lns = "
#
# server address:
server_address=127.0.0.1

nrpe_user=nagios
nrpe_group=nagios

include=/etc/nrpe_local.cfg

command[check_users]=/usr/lib/nagios/check_users -w 5 -c 10
command[check_load]=/usr/lib/nagios/check_load -w 15,10,5 -c 30,25,20
command[check_mongoscl_proc]=/usr/lib64/nagios/plugins/check_procs -c 1:1 --ereg-argument-array=mongosCL
command[test_command]= foo bar \n
include=/etc/nrpe/nrpe.cfg
include_dir=/etc/nrpe/cfgdir/ \n
# trailing whitespaces   \n"

  test Nrpe.lns get lns =
    { }
    { }
    { "#comment" = "server address:" }
    { "server_address" = "127.0.0.1" }
    { }
    { "nrpe_user" = "nagios" }
    { "nrpe_group" = "nagios" }
    { }
    { "include"
      { "file" = "/etc/nrpe_local.cfg" }
    }
    { }
    { "command"
      { "check_users" = "/usr/lib/nagios/check_users -w 5 -c 10" }
    }
    { "command"
      { "check_load" = "/usr/lib/nagios/check_load -w 15,10,5 -c 30,25,20" }
    }
    { "command"
      { "check_mongoscl_proc" = "/usr/lib64/nagios/plugins/check_procs -c 1:1 --ereg-argument-array=mongosCL" }
    }
    { "command"
      { "test_command" = " foo bar " }
    }
    { }
    { "include"
      { "file" = "/etc/nrpe/nrpe.cfg" }
    }
    { "include_dir"
      { "dir" = "/etc/nrpe/cfgdir/" }
    }
    { }
    { "#comment" = "trailing whitespaces" }
