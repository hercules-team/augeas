module Test_modprobe =

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

# Blacklist
blacklist watchdog_drivers  \t

# Install commands
install bar echo Installing bar
install foo echo Installing foo
install export_nodep-$BITNESS echo Installing export_nodep

# Remove commands
remove bar echo Removing bar
remove foo echo Removing foo
remove export_nodep-$BITNESS echo Removing export_nodep

# Softdep
softdep uhci-hcd post: foo
softdep uhci-hcd pre: ehci-hcd foo
softdep uhci-hcd pre: ehci-hcd foo post: foo
"

test Modprobe.lns get conf =
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
  { "#comment" = "Blacklist" }
  { "blacklist" = "watchdog_drivers" }
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
  { "#comment" = "Softdep" }
  { "softdep" = "uhci-hcd"
    { "post" = "foo" }
  }
  { "softdep" = "uhci-hcd"
    { "pre" = "ehci-hcd" }
    { "pre" = "foo" }
  }
  { "softdep" = "uhci-hcd"
    { "pre" = "ehci-hcd" }
    { "pre" = "foo" }
    { "post" = "foo" }
  }


(* eol-comments *)
test Modprobe.lns get "blacklist brokenmodule # never worked\n" =
  { "blacklist" = "brokenmodule"
    { "#comment" = "never worked" } }


(* Ticket 108 *)
let options_space_quote = "options name attr1=\"val\" attr2=\"val2 val3\"\n"

test Modprobe.entry get options_space_quote =
  { "options" = "name"
    { "attr1" = "\"val\"" }
    { "attr2" = "\"val2 val3\"" }
  }

(* Allow spaces around the '=', BZ 826752 *)
test Modprobe.entry get "options ipv6 disable = 1\n" =
  { "options" = "ipv6"
    { "disable" = "1" } }

(* Support multiline split commands, Ubuntu bug #1054306 *)
test Modprobe.lns get "# /etc/modprobe.d/iwlwifi.conf
# iwlwifi will dyamically load either iwldvm or iwlmvm depending on the
# microcode file installed on the system. When removing iwlwifi, first
# remove the iwl?vm module and then iwlwifi.
remove iwlwifi \
(/sbin/lsmod | grep -o -e ^iwlmvm -e ^iwldvm -e ^iwlwifi | xargs /sbin/rmmod) \
&& /sbin/modprobe -r mac80211\n" =
  { "#comment" = "/etc/modprobe.d/iwlwifi.conf" }
  { "#comment" = "iwlwifi will dyamically load either iwldvm or iwlmvm depending on the" }
  { "#comment" = "microcode file installed on the system. When removing iwlwifi, first" }
  { "#comment" = "remove the iwl?vm module and then iwlwifi." }
  { "remove" = "iwlwifi"
    { "command" = "(/sbin/lsmod | grep -o -e ^iwlmvm -e ^iwldvm -e ^iwlwifi | xargs /sbin/rmmod) \\\n&& /sbin/modprobe -r mac80211" }
  }
