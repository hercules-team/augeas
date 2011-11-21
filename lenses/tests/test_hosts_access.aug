module Test_Hosts_Access =

let multi_daemon = "sshd, sendmail : 10.234.\n"

test Hosts_Access.lns get multi_daemon =
  { "1"
    { "process" = "sshd" }
    { "process" = "sendmail" }
    { "client" = "10.234." }
  }

let multi_client = "sshd: 10.234. , 192.168.\n"

test Hosts_Access.lns get multi_client =
  { "1"
    { "process" = "sshd" }
    { "client" = "10.234." }
    { "client" = "192.168." }
  }

let daemon_except = "ALL Except sshd : 10.234.\n"

test Hosts_Access.lns get daemon_except =
  { "1"
    { "process" = "ALL" }
    { "except"
      { "process" = "sshd" }
    }
    { "client" = "10.234." }
  }

let client_except = "sshd : ALL EXCEPT 192.168\n"

test Hosts_Access.lns get client_except =
  { "1"
    { "process" = "sshd" }
    { "client" = "ALL" }
    { "except"
      { "client" = "192.168" }
    }
  }

let daemon_host = "sshd@192.168.0.1: 10.234.\n"

test Hosts_Access.lns get daemon_host =
  { "1"
    { "process" = "sshd"
      { "host" = "192.168.0.1" }
    }
    { "client" = "10.234." }
  }

let user_client = "sshd: root@.example.tld\n"

test Hosts_Access.lns get user_client =
  { "1"
    { "process" = "sshd" }
    { "client" = ".example.tld"
      { "user" = "root" }
    }
  }

let shell_command = "sshd: 192.168. : /usr/bin/my_cmd -t -f some_arg\n"

test Hosts_Access.lns get shell_command =
  { "1"
    { "process" = "sshd" }
    { "client" = "192.168." }
    { "shell_command" = "/usr/bin/my_cmd -t -f some_arg" }
  }

let sample_hosts_allow = "# hosts.allow	This file describes the names of the hosts which are
#		allowed to use the local INET services, as decided
#		by the '/usr/sbin/tcpd' server.
in.telnetd:	192.168.1.
sshd:		70.16., 207.228.
ipop3d:		ALL
sendmail:	ALL
"

test Hosts_Access.lns get sample_hosts_allow =
  { "#comment" = "hosts.allow	This file describes the names of the hosts which are" }
  { "#comment" = "allowed to use the local INET services, as decided" }
  { "#comment" = "by the '/usr/sbin/tcpd' server." }
  { "1"
    { "process" = "in.telnetd" }
    { "client" = "192.168.1." }
  }
  { "2"
    { "process" = "sshd" }
    { "client" = "70.16." }
    { "client" = "207.228." }
  }
  { "3"
    { "process" = "ipop3d" }
    { "client" = "ALL" }
  }
  { "4"
    { "process" = "sendmail" }
    { "client" = "ALL" }
  }


let sample_hosts_deny = "#
# hosts.deny	This file describes the names of the hosts which are
#		*not* allowed to use the local INET services, as decided
#		by the '/usr/sbin/tcpd' server.
in.telnetd: all

sshd: 61., 62., \
 64.179., 65.
"

test Hosts_Access.lns get sample_hosts_deny =
  {  }
  { "#comment" = "hosts.deny	This file describes the names of the hosts which are" }
  { "#comment" = "*not* allowed to use the local INET services, as decided" }
  { "#comment" = "by the '/usr/sbin/tcpd' server." }
  { "1"
    { "process" = "in.telnetd" }
    { "client" = "all" }
  }
  {  }
  { "2"
    { "process" = "sshd" }
    { "client" = "61." }
    { "client" = "62." }
    { "client" = "64.179." }
    { "client" = "65." }
  }
