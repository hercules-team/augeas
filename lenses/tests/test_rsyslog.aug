(*
Module: Test_Rsyslog
  Provides unit tests and examples for the <Rsyslog> lens.
*)

module Test_Rsyslog =

(* Variable: conf *)
let conf = "# rsyslog v5 configuration file

$ModLoad imuxsock # provides support for local system logging (e.g. via logger command)
$ModLoad imklog   # provides kernel logging support (previously done by rklogd)
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
