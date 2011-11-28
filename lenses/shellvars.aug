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
      store (char* | dquot | squot | bquot | empty_array)

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
    generic_cond "if" "@if" "then" (entry+ . elif? . else?) "fi"

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

  let rec rec_entry =
    let entry = comment | empty | source | kv
              | unset | bare_export | builtin | rec_entry in
        cond_if entry
      | loop_for entry
      | loop_select entry
      | loop_while entry
      | loop_until entry
      | case entry

  let lns = (comment | empty | source | kv | unset | bare_export | builtin | rec_entry) *

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

  let filter_ifcfg   = incl "/etc/sysconfig/network-scripts/ifcfg-*"
                     . incl "/etc/sysconfig/network/ifcfg-*"
  let filter_default = incl "/etc/default/*"
  let filter_misc    = incl "/etc/arno-iptables-firewall/debconf.cfg"
                     . incl "/etc/cron-apt/config"
                     . incl "/etc/environment"
                     . incl "/etc/blkid.conf"

  let filter = filter_sysconfig
             . filter_ifcfg
             . filter_default
             . filter_misc
             . Util.stdexcl

  let xfm = transform lns filter

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
