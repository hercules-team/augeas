module Test_syslog =

	let conf="# $FreeBSD: src/etc/syslog.conf,v 1.30.2.1 2009/08/03 08:13:06 kensmith Exp $
# 

daemon.info                                     /var/log/cvsupd.log
security.*					-/var/log/security
*.notice;authpriv.none;kern.debug;lpr.info;mail.crit;news.err	/var/log/messages
uucp,news.crit                                          /var/log/spooler
*.emerg						*
daemon.!info					/var/log/foo
daemon.<=info					/var/log/foo
daemon.!<=info					/var/log/foo
*.*						@syslog.far.away
*.*						@syslog.far.away:123
*.*						@@syslog.far.away
*.*						@@syslog.far.away:123
*.*						foo,bar
*.*						|\"/usr/bin/soft arg\"
!startslip
# get that out
*.*						/var/log/slip.log
!pppd,ppp

*.*						/var/log/ppp.log
!+startslip
*.*						/var/log/slip.log
!-startslip
*.*						/var/log/slip.log
#!pppd
*.*						/var/log/ppp.log
+foo.example.com
daemon.info                                     /var/log/cvsupd.log
+foo.example.com,bar.example.com
daemon.info                                     /var/log/cvsupd.log
#+bar.example.com
daemon.info                                     /var/log/cvsupd.log
-foo.example.com
daemon.info                                     /var/log/cvsupd.log
+*
daemon.info                                     /var/log/cvsupd.log
!*
daemon.info                                     /var/log/cvsupd.log
*.=debug;\
        auth,authpriv.none;\
        news.none;mail.none     -/var/log/debug
"

	test Syslog.lns get conf =
          { "#comment" = "$FreeBSD: src/etc/syslog.conf,v 1.30.2.1 2009/08/03 08:13:06 kensmith Exp $" }
	  { }
	  { }
	  { "entry"
	    { "selector" { "facility" = "daemon" } { "level" = "info" } }
	    { "action" { "file" = "/var/log/cvsupd.log" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "security" } { "level" = "*" } }
	    { "action" { "no_sync" } { "file" = "/var/log/security" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "*" } { "level" = "notice" } }
	    { "selector" { "facility" = "authpriv" } { "level" = "none" } }
	    { "selector" { "facility" = "kern" } { "level" = "debug" } }
	    { "selector" { "facility" = "lpr" } { "level" = "info" } }
	    { "selector" { "facility" = "mail" } { "level" = "crit" } }
	    { "selector" { "facility" = "news" } { "level" = "err" } }
	    { "action" { "file" = "/var/log/messages" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "uucp" } { "facility" = "news" } { "level" = "crit" } }
	    { "action" { "file" = "/var/log/spooler" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "*" } { "level" = "emerg" } }
	    { "action" { "user" = "*" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "daemon" } { "comparison" = "!" } { "level" = "info" } }
	    { "action" { "file" = "/var/log/foo" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "daemon" } { "comparison" = "<=" } { "level" = "info" } }
	    { "action" { "file" = "/var/log/foo" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "daemon" } { "comparison" = "!<=" } { "level" = "info" } }
	    { "action" { "file" = "/var/log/foo" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "*" } { "level" = "*" } }
	    { "action" { "protocol" = "@" } { "hostname" = "syslog.far.away" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "*" } { "level" = "*" } }
	    { "action" { "protocol" = "@" } { "hostname" = "syslog.far.away" } { "port" = "123" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "*" } { "level" = "*" } }
	    { "action" { "protocol" = "@@" } { "hostname" = "syslog.far.away" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "*" } { "level" = "*" } }
	    { "action" { "protocol" = "@@" } { "hostname" = "syslog.far.away" } { "port" = "123" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "*" } { "level" = "*" } }
	    { "action" { "user" = "foo" } { "user" = "bar" } }
	  }
	  { "entry"
	    { "selector" { "facility" = "*" } { "level" = "*" } }
	    { "action" { "program" = "\"/usr/bin/soft arg\"" } }
	  }
	  { "program"
	    { "program" = "startslip" }
	    { "#comment" = "get that out" }
	    { "entry"
	      { "selector" { "facility" = "*" } { "level" = "*" } }
	      { "action" { "file" = "/var/log/slip.log" } }
	    }
	  }
	  { "program"
	    { "program" = "pppd" }
	    { "program" = "ppp" }
	    { }
	    { "entry"
	      { "selector" { "facility" = "*" } { "level" = "*" } }
	      { "action" { "file" = "/var/log/ppp.log" } }
	    }
	  }
	  { "program"
	    { "program" = "startslip" }
	    { "entry"
	      { "selector" { "facility" = "*" } { "level" = "*" } }
	      { "action" { "file" = "/var/log/slip.log" } }
	    }
	  }
	  { "program"
	    { "reverse" }
	    { "program" = "startslip" }
	    { "entry"
	      { "selector" { "facility" = "*" } { "level" = "*" } }
	      { "action" { "file" = "/var/log/slip.log" } }
	    }
	  }
	  { "program"
	    { "program" = "pppd" }
	    { "entry"
	      { "selector" { "facility" = "*" } { "level" = "*" } }
	      { "action" { "file" = "/var/log/ppp.log" } }
	    }
	  }
	  { "hostname"
	    { "hostname" = "foo.example.com" }
	    { "entry"
	      { "selector" { "facility" = "daemon" } { "level" = "info" } }
	      { "action" { "file" = "/var/log/cvsupd.log" } }
	    }
	  }
	  { "hostname"
	    { "hostname" = "foo.example.com" }
	    { "hostname" = "bar.example.com" }
	    { "entry"
	      { "selector" { "facility" = "daemon" } { "level" = "info" } }
	      { "action" { "file" = "/var/log/cvsupd.log" } }
	    }
	  }
	  { "hostname"
	    { "hostname" = "bar.example.com" }
	    { "entry"
	      { "selector" { "facility" = "daemon" } { "level" = "info" } }
	      { "action" { "file" = "/var/log/cvsupd.log" } }
	    }
	  }
	  { "hostname"
	    { "reverse" }
	    { "hostname" = "foo.example.com" }
	    { "entry"
	      { "selector" { "facility" = "daemon" } { "level" = "info" } }
	      { "action" { "file" = "/var/log/cvsupd.log" } }
	    }
	  }
	  { "hostname"
	    { "hostname" = "*" }
	    { "entry"
	      { "selector" { "facility" = "daemon" } { "level" = "info" } }
	      { "action" { "file" = "/var/log/cvsupd.log" } }
	    }
	  }
	  { "program"
	    { "program" = "*" }
	    { "entry"
	      { "selector" { "facility" = "daemon" } { "level" = "info" } }
	      { "action" { "file" = "/var/log/cvsupd.log" } } }
            { "entry"
              { "selector" { "facility" = "*" } { "comparison" = "=" } { "level" = "debug" } }
              { "selector" { "facility" = "auth" } { "facility" = "authpriv" } { "level" = "none" } }
              { "selector" { "facility" = "news" } { "level" = "none" } }
              { "selector" { "facility" = "mail" } { "level" = "none" } }
	      { "action" { "no_sync" } { "file" = "/var/log/debug" } } }
	  }

	(* changing file *)
	test Syslog.lns put "*.* /var\n" after
	  set "/entry[1]/action/file" "/foo"
	  = "*.* /foo\n"

	(* removing entry *)
	test Syslog.lns put "*.* /var\n" after
	  rm "/entry[1]"
	  = ""

	(* changing facility and level *)
	test Syslog.lns put "*.* /var\n" after
	  set "/entry[1]/selector/facility" "daemon" ;
	  set "/entry[1]/selector/level" "info"
	  = "daemon.info /var\n"

	(* insert a facility *)
	test Syslog.lns put "daemon.* /var\n" after
	  insa "facility" "/entry/selector/facility" ;
	  set "/entry/selector/facility[2]" "mail"
	  = "daemon,mail.* /var\n"

	(* creating an entry *)
	test Syslog.lns put "" after
	  set "/entry/selector/facility" "daemon" ;
	  set "/entry/selector/level" "info" ;
	  set "/entry/action/file" "/var"
	  = "daemon.info\t/var\n"

	(* inserting an entry before *)
	test Syslog.lns put "*.* /var\n" after
	  insb "entry" "/entry" ;
	  set "/entry[1]/selector/facility" "daemon" ;
	  set "/entry[1]/selector/level" "info" ;
	  set "/entry[1]/action/file" "/foo"
	  = "daemon.info /foo\n*.*\t/var\n"

	(* inserting an entry after *)
	test Syslog.lns put "*.* /var\n" after
	  insa "entry" "/entry" ;
	  set "/entry[2]/selector/facility" "daemon" ;
	  set "/entry[2]/selector/level" "info" ;
	  set "/entry[2]/action/file" "/foo"
	  = "*.* /var\ndaemon.info\t/foo\n"

	(* insert sync on a file *)
	test Syslog.lns put "*.* /var\n" after
	  insb "no_sync" "/entry/action/file"
	  = "*.* -/var\n"

	(* changing file to remote host *)
	test Syslog.lns put "*.* /var\n" after
	  rm "/entry/action/file" ;
	  set "/entry/action/protocol"  "@" ;
	  set "/entry/action/hostname" "far.far.away"
	  = "*.* @far.far.away\n"

	(* changing file to remote host *)
	test Syslog.lns put "*.* /var/lib\n" after
	  rm "/entry/action/file" ;
	  set "/entry/action/protocol"  "@@" ;
	  set "/entry/action/hostname" "far.far.away"
	  = "*.* @@far.far.away\n"

	(* changing file to * *)
	test Syslog.lns put "*.* /var\n" after
	  rm "/entry/action/file" ;
	  set "/entry/action/user" "*"
	  = "*.* *\n"

	(* changing file to users *)
	test Syslog.lns put "*.* /var\n" after
	  rm "/entry/action/file" ;
	  set "/entry/action/user[1]" "john" ;
	  set "/entry/action/user[2]" "paul" ;
	  set "/entry/action/user[3]" "george" ;
	  set "/entry/action/user[4]" "ringo"
	  = "*.* john,paul,george,ringo\n"

	(* changing file to program *)
	test Syslog.lns put "*.* /var\n" after
	  rm "/entry/action/file" ;
	  set "/entry/action/program" "/usr/bin/foo"
	  = "*.* |/usr/bin/foo\n"

	(* inserting a matching program *)
	test Syslog.lns put "" after
	  insa "program" "/" ;
	  set "/program/program" "foo"
	  = "!foo\n"

	(* inserting an entry to a matching program *)
	test Syslog.lns put "!foo\n" after
	  set "/program/entry/selector/facility" "*" ;
	  set "/program/entry/selector/level" "*" ;
	  set "/program/entry/action/file" "/foo"
	  = "!foo\n*.*\t/foo\n"

	(* inserting a matching hostname *)
	test Syslog.lns put "" after
	  insa "hostname" "/" ;
	  set "/hostname/hostname" "foo.foo.away"
	  = "+foo.foo.away\n"

	(* inserting an entry to a matching hostname *)
	test Syslog.lns put "+foo.foo.away\n" after
	  set "/hostname/entry/selector/facility" "*" ;
	  set "/hostname/entry/selector/level" "*" ;
	  set "/hostname/entry/action/file" "/foo"
	  = "+foo.foo.away\n*.*\t/foo\n"

	(* inserting a reverse matching hostname *)
	test Syslog.lns put "" after
	  insa "hostname" "/" ;
	  set "/hostname/reverse" "" ;
	  set "/hostname/hostname" "foo.foo.away"
	  = "-foo.foo.away\n"

        (* tokens can contain capital letters *)
        test Syslog.lns get "LOCAL5.*    -/var/log/foo.log\n" =
          { "entry"
            { "selector"
              { "facility" = "LOCAL5" }
              { "level" = "*" }
            }
            { "action"
              { "no_sync" }
              { "file" = "/var/log/foo.log" }
            }
          }

