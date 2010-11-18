(* Generic lens for shell-script config files like the ones found *)
(* in /etc/sysconfig                                              *)
module Shellvars =
  autoload xfm

  let eol = Util.eol

  let key_re = /[A-Za-z0-9_]+(\[[0-9]+\])?/ - "unset" - "export"
  let eq = Util.del_str "="
  let comment = Util.comment
  let comment_or_eol = Util.comment_or_eol
  let empty   = Util.empty
  let xchgs   = Build.xchgs

  let char  = /[^#() '"\t\n]|\\\\"/
  let dquot = /"([^"\\\n]|\\\\.)*"/                    (* " Emacs, relax *)
  let squot = /'[^'\n]*'/
  (* For some reason, `` conflicts with comment_or_eol *)
  let bquot = /`[^#`\n]*`/

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
      store (char* | dquot | squot | bquot | empty_array)

  let export = [ key "export" . Util.del_ws_spc ]
  let kv = [ export? . key key_re . eq . (simple_value | array) . comment_or_eol ]

  let var_action (name:string) =
    [ xchgs name ("@" . name) . Util.del_ws_spc . store key_re . comment_or_eol ]

  let unset = var_action "unset"
  let bare_export = var_action "export"

  let source =
    [
      del /\.|source/ "." . label ".source" .
      Util.del_ws_spc . store /[^= \t\n]+/ . eol
    ]

  let shell_builtin_cmds = "ulimit"

  let builtin =
    [ label "@builtin"
      . store shell_builtin_cmds
      . Util.del_ws_spc
      . [ label "args" . store /[^ \t\n][^;\n]+[^ \t\n]|[^ \t;\n]+/ ]
      . eol ]

  let lns = (comment | empty | source | kv | unset | bare_export | builtin) *

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
      sc_incl "spamassassin" .
      sc_incl "suseconfig" .
      sc_incl "sysstat" .
      sc_incl "system-config-users" .
      sc_incl "vncservers" .
      sc_incl "wpa_supplicant" .
      sc_incl "xend" .
      sc_incl "xendomains"

  let filter_ifcfg   = incl "/etc/sysconfig/network-scripts/ifcfg-*"
                     . incl "/etc/sysconfig/network/ifcfg-*"
  let filter_default = incl "/etc/default/*"
  let filter_misc    = incl "/etc/arno-iptables-firewall/debconf.cfg"
                     . incl "/etc/cron-apt/config"
                     . incl "/etc/environment"

  let filter = filter_sysconfig
             . filter_ifcfg
             . filter_default
             . filter_misc
             . Util.stdexcl

  let xfm = transform lns filter

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
