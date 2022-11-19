(*
Module: Test_Sos
  Provides unit tests and examples for the <Sos> lens.
*)

module Test_Sos =

let sos_conf_example = "# Sample /etc/sos/sos.conf

[global]
# Set global options here
verbose = 3
verify=yes

[report]
# Options for any `sos report`
skip-plugins = rpm, selinux, dovecot
enable-plugins = host,logs

[collect]
# Options for `sos collect`
ssh-key = /home/user/.ssh/mykey
password = true

[clean]
# Options for `sos clean|mask`
domains = mydomain.com

[plugin_options]
rpm.rpmva = off
"

test Sos.lns get sos_conf_example =
  { "#comment" = "Sample /etc/sos/sos.conf" }
  { }
  { "global"
    { "#comment" = "Set global options here" }
    { "verbose" = "3" }
    { "verify" = "yes" }
    { }
  }
  { "report"
    { "#comment" = "Options for any `sos report`" }
    { "skip-plugins" = "rpm, selinux, dovecot" }
    { "enable-plugins" = "host,logs" }
    { }
  }
  { "collect"
    { "#comment" = "Options for `sos collect`" }
    { "ssh-key" = "/home/user/.ssh/mykey" }
    { "password" = "true" }
    { }
  }
  { "clean"
    { "#comment" = "Options for `sos clean|mask`" }
    { "domains" = "mydomain.com" }
    { }
  }
  { "plugin_options"
    { "rpm.rpmva" = "off" }
  }

