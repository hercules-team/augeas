(*
Module: Test_Rsyslog
  Provides unit tests and examples for the <Rsyslog> lens.
*)

module Test_Rsyslog =

(* Variable: conf *)
let conf = "# rsyslog v5 configuration file

$ModLoad imuxsock # provides support for local system logging (e.g. via logger command)
$ModLoad imklog   # provides kernel logging support (previously done by rklogd)
module(load=\"immark\" markmessageperiod=\"60\" fakeoption=\"bar\") #provides --MARK-- message capability

timezone(id=\"CET\" offset=\"+01:00\")

$UDPServerRun 514
$InputTCPServerRun 514
$ActionFileDefaultTemplate RSYSLOG_TraditionalFileFormat
$ActionFileEnableSync on
$IncludeConfig /etc/rsyslog.d/*.conf

*.info;mail.none;authpriv.none;cron.none                /var/log/messages
authpriv.*                                              /var/log/secure
*.emerg                                                 *
*.*    @2.7.4.1
*.*    @@2.7.4.1
*.emerg :omusrmsg:*
*.emerg :omusrmsg:foo,bar
*.emerg | /dev/xconsole
"

(* Test: Rsyslog.lns *)
test Rsyslog.lns get conf =
  { "#comment" = "rsyslog v5 configuration file" }
  {  }
  { "$ModLoad" = "imuxsock"
    { "#comment" = "provides support for local system logging (e.g. via logger command)" }
  }
  { "$ModLoad" = "imklog"
    { "#comment" = "provides kernel logging support (previously done by rklogd)" }
  }
  { "module"
    { "load" = "immark" }
    { "markmessageperiod" = "60" }
    { "fakeoption" = "bar" }
    { "#comment" = "provides --MARK-- message capability" }
  }
  {  }
  { "timezone"
    { "id" = "CET" }
    { "offset" = "+01:00" }
  }
  {  }
  { "$UDPServerRun" = "514" }
  { "$InputTCPServerRun" = "514" }
  { "$ActionFileDefaultTemplate" = "RSYSLOG_TraditionalFileFormat" }
  { "$ActionFileEnableSync" = "on" }
  { "$IncludeConfig" = "/etc/rsyslog.d/*.conf" }
  {  }
  { "entry"
    { "selector"
      { "facility" = "*" }
      { "level" = "info" }
    }
    { "selector"
      { "facility" = "mail" }
      { "level" = "none" }
    }
    { "selector"
      { "facility" = "authpriv" }
      { "level" = "none" }
    }
    { "selector"
      { "facility" = "cron" }
      { "level" = "none" }
    }
    { "action"
      { "file" = "/var/log/messages" }
    }
  }
  { "entry"
    { "selector"
      { "facility" = "authpriv" }
      { "level" = "*" }
    }
    { "action"
      { "file" = "/var/log/secure" }
    }
  }
  { "entry"
    { "selector"
      { "facility" = "*" }
      { "level" = "emerg" }
    }
    { "action"
      { "user" = "*" }
    }
  }
  { "entry"
    { "selector"
      { "facility" = "*" }
      { "level" = "*" }
    }
    { "action"
      { "protocol" = "@" }
      { "hostname" = "2.7.4.1" }
    }
  }
  { "entry"
    { "selector"
      { "facility" = "*" }
      { "level" = "*" }
    }
    { "action"
      { "protocol" = "@@" }
      { "hostname" = "2.7.4.1" }
    }
  }
  { "entry"
    { "selector"
      { "facility" = "*" }
      { "level" = "emerg" }
    }
    { "action"
      { "omusrmsg" = "*" }
    }
  }
  { "entry"
    { "selector"
      { "facility" = "*" }
      { "level" = "emerg" }
    }
    { "action"
      { "omusrmsg" = "foo" }
      { "omusrmsg" = "bar" } }
  }
  { "entry"
    { "selector"
      { "facility" = "*" }
      { "level" = "emerg" }
    }
    { "action"
      { "pipe" = "/dev/xconsole" }
    }
  }

(* Parse complex $template lines, RHBZ#1083016 *)
test Rsyslog.lns get "$template SpiceTmpl,\"%TIMESTAMP%.%TIMESTAMP:::date-subseconds% %syslogtag% %syslogseverity-text%:%msg:::sp-if-no-1st-sp%%msg:::drop-last-lf%\\n\"\n" =
  { "$template" = "SpiceTmpl,\"%TIMESTAMP%.%TIMESTAMP:::date-subseconds% %syslogtag% %syslogseverity-text%:%msg:::sp-if-no-1st-sp%%msg:::drop-last-lf%\\n\"" }

(* Parse property-based filters, RHBZ#1083016 *)
test Rsyslog.lns get ":programname, startswith, \"spice-vdagent\" /var/log/spice-vdagent.log;SpiceTmpl\n" =
  { "filter"
    { "property" = "programname" }
    { "operation" = "startswith" }
    { "value" = "spice-vdagent" }
    { "action"
      { "file" = "/var/log/spice-vdagent.log" }
      { "template" = "SpiceTmpl" } } }

test Rsyslog.lns get ":msg, !contains, \"error\" /var/log/noterror.log\n" =
  { "filter"
    { "property" = "msg" }
    { "operation" = "!contains" }
    { "value" = "error" }
    { "action"
      { "file" = "/var/log/noterror.log" } } }

test Rsyslog.lns get ":msg,!contains,\"garbage\" ~\n" =
  { "filter"
    { "property" = "msg" }
    { "operation" = "!contains" }
    { "value" = "garbage" }
    { "action"
      { "discard" } } }

test Rsyslog.lns put "" after
  set "/module[1]/load" "imuxsock"
  = "module(load=\"imuxsock\")\n"

test Rsyslog.lns put "" after
  set "/module[1]/load" "imuxsock" ;
  set "/module[1]/SysSock.RateLimit.Interval" "0"
  = "module(load=\"imuxsock\" SysSock.RateLimit.Interval=\"0\")\n"

test Rsyslog.lns put "" after
  set "/module[1]/load" "imuxsock" ;
  set "/module[1]/SysSock.RateLimit.Interval" "0" ;
  set "/module[1]/SysSock.RateLimit.Burst" "1"
  = "module(load=\"imuxsock\" SysSock.RateLimit.Interval=\"0\" SysSock.RateLimit.Burst=\"1\")\n"

(* On Fedora 26, there are comments in module statements *)
test Rsyslog.lns get "module(load=\"imuxsock\" 	  # provides support for local system logging (e.g. via logger command)
       SysSock.Use=\"off\") # Turn off message reception via local log socket;
			  # local messages are retrieved through imjournal now.\n" =
  { "module"
    { "load" = "imuxsock" }
    { "SysSock.Use" = "off" }
    { "#comment" = "Turn off message reception via local log socket;" } }
  { "#comment" = "local messages are retrieved through imjournal now." }
