module Test_modules_conf =

(* Based on 04config.sh from module-init-tools *)

let conf = "# Various aliases
alias alias_to_foo foo
alias alias_to_bar bar
alias alias_to_export_dep-$BITNESS export_dep-$BITNESS

# Various options, including options to aliases.
options alias_to_export_dep-$BITNESS I am alias to export_dep
options alias_to_noexport_nodep-$BITNESS_with_tabbed_options index=0 id=\"Thinkpad\" isapnp=0 \\
\tport=0x530 cport=0x538 fm_port=0x388 \\
\tmpu_port=-1 mpu_irq=-1 \\
\tirq=9 dma1=1 dma2=3 \\
\tenable=1 isapnp=0

# Install commands
install bar echo Installing bar
install foo echo Installing foo
install export_nodep-$BITNESS echo Installing export_nodep

# Pre- and post- install something
pre-install ide-scsi modprobe ide-cd # load ide-cd before ide-scsi
post-install serial /etc/init.d/setserial modload > /dev/null 2> /dev/null

# Remove commands
remove bar echo Removing bar
remove foo echo Removing foo
remove export_nodep-$BITNESS echo Removing export_nodep

#Pre- and post- remove something
pre-remove serial /etc/init.d/setserial modsave  > /dev/null 2> /dev/null
post-remove bttv rmmod tuner

# Misc other directives
probeall /dev/cdroms
keep
path=/lib/modules/`uname -r`/alsa
"

test Modules_conf.lns get conf =
  { "#comment" = "Various aliases" }
  { "alias" = "alias_to_foo"
    { "modulename" = "foo" }
  }
  { "alias" = "alias_to_bar"
    { "modulename" = "bar" }
  }
  { "alias" = "alias_to_export_dep-$BITNESS"
    { "modulename" = "export_dep-$BITNESS" }
  }
  {  }
  { "#comment" = "Various options, including options to aliases." }
  { "options" = "alias_to_export_dep-$BITNESS"
    { "I" }
    { "am" }
    { "alias" }
    { "to" }
    { "export_dep" }
  }
  { "options" = "alias_to_noexport_nodep-$BITNESS_with_tabbed_options"
    { "index" = "0" }
    { "id" = "\"Thinkpad\"" }
    { "isapnp" = "0" }
    { "port" = "0x530" }
    { "cport" = "0x538" }
    { "fm_port" = "0x388" }
    { "mpu_port" = "-1" }
    { "mpu_irq" = "-1" }
    { "irq" = "9" }
    { "dma1" = "1" }
    { "dma2" = "3" }
    { "enable" = "1" }
    { "isapnp" = "0" }
  }
  {  }
  { "#comment" = "Install commands" }
  { "install" = "bar"
    { "command" = "echo Installing bar" }
  }
  { "install" = "foo"
    { "command" = "echo Installing foo" }
  }
  { "install" = "export_nodep-$BITNESS"
    { "command" = "echo Installing export_nodep" }
  }
  {  }
  { "#comment" = "Pre- and post- install something" }
  { "pre-install" = "ide-scsi"
    { "command" = "modprobe ide-cd" }
    { "#comment" = "load ide-cd before ide-scsi" }
  }
  { "post-install" = "serial"
    { "command" = "/etc/init.d/setserial modload > /dev/null 2> /dev/null" }
  }
  {  }
  { "#comment" = "Remove commands" }
  { "remove" = "bar"
    { "command" = "echo Removing bar" }
  }
  { "remove" = "foo"
    { "command" = "echo Removing foo" }
  }
  { "remove" = "export_nodep-$BITNESS"
    { "command" = "echo Removing export_nodep" }
  }
  {  }
  { "#comment" = "Pre- and post- remove something" }
  { "pre-remove" = "serial"
    { "command" = "/etc/init.d/setserial modsave  > /dev/null 2> /dev/null" }
  }
  { "post-remove" = "bttv"
    { "command" = "rmmod tuner" }
  }
  {  }
  { "#comment" = "Misc other directives" }
  { "probeall" = "/dev/cdroms" }
  { "keep" }
  { "path" = "/lib/modules/`uname -r`/alsa" }
