(*
Module: Shellvars
 Generic lens for shell-script config files like the ones found
 in /etc/sysconfig

About: License
   This file is licenced under the LGPLv2+, like the rest of Augeas.

About: Lens Usage
   To be documented
*)

module Shellvars =
  autoload xfm

  let eol = del /[ \t]+|[ \t]*[;\n]/ "\n"
  let semicol_eol = del /[ \t]*[;\n]/ "\n"

  let key_re = /[A-Za-z0-9_]+(\[[0-9]+\])?/ - "unset" - "export"
  let eq = Util.del_str "="

  let comment = Util.comment
  let comment_eol = Util.comment_eol
  let comment_or_eol = comment_eol | semicol_eol

  let empty   = Util.empty
  let xchgs   = Build.xchgs
  let semicol = del /;?/ ""

  let char  = /[^;#() '"\t\n]|\\\\"/
  let dquot = /"([^"\\\n]|\\\\.)*"/                    (* " Emacs, relax *)
  let squot = /'[^'\n]*'/
  (* For some reason, `` conflicts with comment_or_eol *)
  let bquot = /`[^#`\n]*`/
  let dollar_assign = /\$\([^#`\n]*\)/

  let sto_to_semicol = store /[^#; \t\n][^#;\n]+[^#; \t\n]|[^#; \t\n]+/

  (* Array values of the form '(val1 val2 val3)'. We do not handle empty *)
  (* arrays here because of typechecking headaches. Instead, they are    *)
  (* treated as a simple value                                           *)
  let array =
    let array_value = store (char+ | dquot) in
    del /\([ \t]*/ "(" . counter "values" .
      [ seq "values" . array_value ] .
      [ del /[ \t\n]+/ " " . seq "values" . array_value ] *
      . del /[ \t]*\)/ ")"

  (* Treat an empty list () as a value '()'; that's not quite correct *)
  (* but fairly close.                                                *)
  let simple_value =
    let empty_array = /\([ \t]*\)/ in
      store (char* | dquot | squot | bquot | dollar_assign | empty_array)

  let export = [ key "export" . Util.del_ws_spc ]
  let kv = [ Util.indent . export? . key key_re
           . eq . (simple_value | array) . semicol . comment_or_eol ]

  let var_action (name:string) =
    [ Util.indent . xchgs name ("@" . name) . Util.del_ws_spc
    . store key_re . semicol . comment_or_eol ]

  let unset = var_action "unset"
  let bare_export = var_action "export"

  let source =
    [ Util.indent
      . del /\.|source/ "." . label ".source"
      . Util.del_ws_spc . store /[^;=# \t\n]+/ . comment_or_eol ]

  let shell_builtin_cmds = "ulimit"

  let builtin =
    [ Util.indent . label "@builtin"
      . store shell_builtin_cmds
      . Util.del_ws_spc
      . [ label "args" . sto_to_semicol ]
      . comment_or_eol ]

  let keyword (kw:string) = Util.indent . Util.del_str kw
  let keyword_label (kw:string) (lbl:string) = keyword kw . label lbl

  let return =
    [ Util.indent . label "@return"
      . Util.del_str "return"
      . ( Util.del_ws_spc . store Rx.integer )?
      . comment_or_eol ]


(************************************************************************
 * Group:                 CONDITIONALS AND LOOPS
 *************************************************************************)

  let generic_cond_start (start_kw:string) (lbl:string)
                         (then_kw:string) (contents:lens) =
      keyword_label start_kw lbl . Sep.space
      . sto_to_semicol . semicol_eol
      . keyword then_kw . eol
      . contents

  let generic_cond (start_kw:string) (lbl:string)
                       (then_kw:string) (contents:lens) (end_kw:string) =
      [ generic_cond_start start_kw lbl then_kw contents
        . keyword end_kw . comment_or_eol ]

  let cond_if (entry:lens) =
    let elif = [ generic_cond_start "elif" "@elif" "then" entry+ ] in
    let else = [ keyword_label "else" "@else" . eol . entry+ ] in
    generic_cond "if" "@if" "then" (entry+ . elif* . else?) "fi"

  let loop_for (entry:lens) =
    generic_cond "for" "@for" "do" entry+ "done"

  let loop_while (entry:lens) =
    generic_cond "while" "@while" "do" entry+ "done"

  let loop_until (entry:lens) =
    generic_cond "until" "@until" "do" entry+ "done"

  let loop_select (entry:lens) =
    generic_cond "select" "@select" "do" entry+ "done"

  let case (entry:lens) =
    let case_entry = [ label "@case_entry"
                       . Util.indent . store /[^ \t\n\)]+/
                       . Util.del_str ")" . eol
                       . entry+
                       . Util.indent . Util.del_str ";;" . eol ] in
      [ keyword_label "case" "@case" . Sep.space
        . store char+
        . del /[ \t\n]+/ " " . Util.del_str "in" . eol
        . case_entry+
        . keyword "esac" . comment_or_eol ]

  let function (entry:lens) =
    [ Util.indent . label "@function"
      . del /(function[ \t]+)?/ ""
      . store Rx.word . del /[ \t]*\(\)/ "()"
      . eol . Util.del_str "{" . eol
      . entry+
      . Util.indent . Util.del_str "}" . eol ]

  let rec rec_entry =
    let entry = comment | empty | source | kv
              | unset | bare_export | builtin | return | rec_entry in
        cond_if entry
      | loop_for entry
      | loop_select entry
      | loop_while entry
      | loop_until entry
      | case entry
      | function entry

  let lns = (comment | empty | source | kv | unset | bare_export | builtin | return | rec_entry) *

  let sc_incl (n:string) = (incl ("/etc/sysconfig/" . n))
  let filter_sysconfig =
      sc_incl "atd" .
      sc_incl "authconfig" .
      sc_incl "autofs" .
      sc_incl "clock" .
      sc_incl "cpuspeed" .
      sc_incl "crond" .
      sc_incl "crontab" .
      sc_incl "desktop" .
      sc_incl "firstboot" .
      sc_incl "grub" .
      sc_incl "hsqldb" .
      sc_incl "httpd" .
      sc_incl "i18n" .
      sc_incl "init" .
      sc_incl "iptables-config" .
      sc_incl "irda" .
      sc_incl "irqbalance" .
      sc_incl "kdump" .
      sc_incl "keyboard" .
      sc_incl "kudzu" .
      sc_incl "libvirtd" .
      sc_incl "lircd" .
      sc_incl "nasd" .
      sc_incl "netconsole" .
      sc_incl "network" .
      sc_incl "nfs" .
      sc_incl "ntpd" .
      sc_incl "prelink" .
      sc_incl "puppet" .
      sc_incl "puppetmaster" .
      sc_incl "readonly-root" .
      sc_incl "rsyslog" .
      sc_incl "samba" .
      sc_incl "saslauthd" .
      sc_incl "selinux" .
      sc_incl "sendmail" .
      sc_incl "smartmontools" .
      sc_incl "snmpd" .
      sc_incl "snmpd.options" .
      sc_incl "snmptrapd" .
      sc_incl "snmptrapd.options" .
      sc_incl "spamassassin" .
      sc_incl "suseconfig" .
      sc_incl "sysstat" .
      sc_incl "system-config-users" .
      sc_incl "vncservers" .
      sc_incl "wpa_supplicant" .
      sc_incl "xend" .
      sc_incl "xendomains"

  let filter_sysconfig_suse =
      sc_incl "abuild" .
      sc_incl "add-on-creator" .
      sc_incl "amavis" .
      sc_incl "amazon" .
      sc_incl "apache2" .
      sc_incl "argus" .
      sc_incl "arpwatch" .
      sc_incl "auditd" .
      sc_incl "autoinstall" .
      sc_incl "autoupdate" .
      sc_incl "backup" .
      sc_incl "bigsister" .
      sc_incl "bluetooth" .
      sc_incl "boot" .
      sc_incl "bootloader" .
      sc_incl "bootsplash" .
      sc_incl "cgred" .
      sc_incl "console" .
      sc_incl "cron" .
      sc_incl "ctdb" .
      sc_incl "cups" .
      sc_incl "ddclient" .
      sc_incl "dhcpd" .
      sc_incl "dhcrelay" .
      sc_incl "displaymanager" .
      sc_incl "dmraid" .
      sc_incl "dracd" .
      sc_incl "esound" .
      sc_incl "fam" .
      sc_incl "festival" .
      sc_incl "fetchmail" .
      sc_incl "fonts-config" .
      sc_incl "git-daemon" .
      sc_incl "gpsd" .
      sc_incl "hardware" .
      sc_incl "hardware/*" .
      sc_incl "icecream" .
      sc_incl "ide" .
      sc_incl "infiniband" .
      sc_incl "inputattach" .
      sc_incl "ipmi" .
      sc_incl "ipvsadm" .
      sc_incl "ispell" .
      sc_incl "joystick" .
      sc_incl "kernel-tunables" .
      sc_incl "language" .
      sc_incl "ldap" .
      sc_incl "lighttpd" .
      sc_incl "lirc" .
      sc_incl "locate" .
      sc_incl "lvm" .
      sc_incl "mail" .
      sc_incl "mailman" .
      sc_incl "mdadm" .
      sc_incl "media-changer" .
      sc_incl "memcached" .
      sc_incl "mouse" .
      sc_incl "mpi-selector" .
      sc_incl "mysql" .
      sc_incl "nagios" .
      sc_incl "named" .
      sc_incl "ncpfs" .
      sc_incl "net-snmp" .
      sc_incl "network/config" .
      sc_incl "network/dhcp" .
      sc_incl "network/dhcp6r" .
      sc_incl "network/dhcp6s" .
      sc_incl "network/ifcfg-*" .
      sc_incl "network/if-down.d/*" .
      sc_incl "network/ifroute-*" .
      sc_incl "network/if-up.d/*" .
      sc_incl "network/providers/*" .
      sc_incl "news" .
      sc_incl "ntp" .
      sc_incl "obs-server" .
      sc_incl "obs-worker" .
      sc_incl "openldap" .
      sc_incl "opensm" .
      sc_incl "osa-dispatcher" .
      sc_incl "pm-profiler" .
      sc_incl "portmap" .
      sc_incl "postfix" .
      sc_incl "postgresql" .
      sc_incl "product-creator" .
      sc_incl "proxy" .
      sc_incl "ptpd" .
      sc_incl "radvd" .
      sc_incl "readonlyroot" .
      sc_incl "rpcbind" .
      sc_incl "sapconf" .
      sc_incl "scpm" .
      sc_incl "seccheck" .
      sc_incl "security" .
      sc_incl "services" .
      sc_incl "shutdown" .
      sc_incl "smt-client" .
      sc_incl "sound" .
      sc_incl "spamd" .
      sc_incl "squid" .
      sc_incl "ssh" .
      sc_incl "storage" .
      sc_incl "SuSEfirewall2" .
      sc_incl "SuSEfirewall2.d/cobbler" .
      sc_incl "SuSEfirewall2.d/services/*" .
      sc_incl "susehelp" .
      sc_incl "suse_register" .
      sc_incl "sysctl" .
      sc_incl "syslog" .
      sc_incl "texlive" .
      sc_incl "ulimit" .
      sc_incl "websphere-as_ce-2.1" .
      sc_incl "windowmanager" .
      sc_incl "wondershaper" .
      sc_incl "words" .
      sc_incl "yast2" .
      sc_incl "ypbind" .
      sc_incl "ypserv"

  let filter_sysconfig_rhn =
      sc_incl "rhn/allowed-actions/*" .
      sc_incl "rhn/allowed-actions/script/*" .
      sc_incl "rhn/clientCaps.d/*" .
      sc_incl "rhn/osad-auth.conf" .
      sc_incl "rhn/osad.conf" .
      sc_incl "rhn/rhncfg-client.conf" .
      sc_incl "rhn/rhncfg-manager.conf" .
      sc_incl "rhn/rhnpushrc" .
      sc_incl "rhn/rhnsd" .
      sc_incl "rhn/up2date" .
      sc_incl "rhn/virt" .
      sc_incl "rhn/virt/auto"

  let filter_ifcfg   = incl "/etc/sysconfig/network-scripts/ifcfg-*"
                     . incl "/etc/sysconfig/network/ifcfg-*"
  let filter_default = incl "/etc/default/*"
  let filter_misc    = incl "/etc/arno-iptables-firewall/debconf.cfg"
                     . incl "/etc/cron-apt/config"
                     . incl "/etc/environment"
                     . incl "/etc/blkid.conf"
                     . incl "/etc/adduser.conf"
                     . incl "/etc/cowpoke.conf"
                     . incl "/etc/cvs-cron.conf"
                     . incl "/etc/cvs-pserver.conf"
                     . incl "/etc/devscripts.conf"
                     . incl "/etc/lintianrc"
                     . incl "/etc/lsb-release"
                     . incl "/etc/popularity-contest.conf"
                     . incl "/etc/ucf.conf"
                     . incl "/etc/updatedb.conf"

  let filter = filter_sysconfig
             . filter_sysconfig_suse
             . filter_sysconfig_rhn
             . filter_ifcfg
             . filter_default
             . filter_misc
             . Util.stdexcl

  let xfm = transform lns filter

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
