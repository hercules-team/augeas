(*
Module: Shellvars
 Generic lens for shell-script config files like the ones found
 in /etc/sysconfig

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented
*)

module Shellvars =
  autoload xfm

  let empty   = Util.empty
  let empty_part_re = Util.empty_generic_re . /\n+/
  let eol = del (/[ \t]+|[ \t]*[;\n]/ . empty_part_re*) "\n"
  let semicol_eol = del (/[ \t]*[;\n]/ . empty_part_re*) "\n"

  let key_re = /[A-Za-z0-9_]+(\[[0-9]+\])?/ - ("unset" | "export")
  let matching_re = "${!" . key_re . /[\*@]\}/
  let eq = Util.del_str "="

  let eol_for_comment = del /([ \t]*\n)([ \t]*(#[ \t]*)?\n)*/ "\n"
  let comment = Util.comment_generic_seteol /[ \t]*#[ \t]*/ " # " eol_for_comment
  (* comment_eol in shell MUST begin with a space *)
  let comment_eol = Util.comment_generic_seteol /[ \t]+#[ \t]*/ " # " eol_for_comment
  let comment_or_eol = comment_eol | semicol_eol

  let xchgs   = Build.xchgs
  let semicol = del /;?/ ""

  let char  = /[^`;() '"\t\n]|\\\\"/
  let dquot = 
       let char = /[^"\\]|\\\\./ | Rx.cl
    in "\"" . char* . "\""                    (* " Emacs, relax *)
  let squot = /'[^']*'/
  let bquot = /`[^`\n]*`/
  (* dbquot don't take spaces or semi-colons *)
  let dbquot = /``[^` \t\n;]+``/
  let dollar_assign = /\$\([^\)#\n]*\)/

  let sto_to_semicol = store /[^#; \t\n][^#;\n]+[^#; \t\n]|[^#; \t\n]+/

  (* Array values of the form '(val1 val2 val3)'. We do not handle empty *)
  (* arrays here because of typechecking headaches. Instead, they are    *)
  (* treated as a simple value                                           *)
  let array =
    let array_value = store (char+ | (dquot | squot)+ | bquot | dbquot | dollar_assign) in
    del /\([ \t]*/ "(" . counter "values" .
      [ seq "values" . array_value ] .
      [ del /[ \t\n]+/ " " . seq "values" . array_value ] *
      . del /[ \t]*\)/ ")"

  (* Treat an empty list () as a value '()'; that's not quite correct *)
  (* but fairly close.                                                *)
  let simple_value =
    let empty_array = /\([ \t]*\)/ in
      store (char* | (dquot | squot)+
            | bquot | dbquot | dollar_assign | empty_array)

  let export = [ key "export" . Util.del_ws_spc ]
  let kv = [ Util.indent . export? . key key_re
           . eq . (simple_value | array) . comment_or_eol ]

  let var_action (name:string) =
    Util.indent . del name name . Util.del_ws_spc .
      [ label ("@" . name) . counter "var_action"
        . Build.opt_list [ seq "var_action" . store (key_re | matching_re) ] Util.del_ws_spc
        . comment_or_eol ]

  let unset = var_action "unset"
  let bare_export = var_action "export"

  let source =
    [ Util.indent
      . del /\.|source/ "." . label ".source"
      . Util.del_ws_spc . store /[^;=# \t\n]+/ . comment_or_eol ]

  let shell_builtin_cmds = "ulimit" | "shift" | "exit"

  let builtin =
    [ Util.indent . label "@builtin"
      . store shell_builtin_cmds
      . (Util.del_ws_spc
      . [ label "args" . sto_to_semicol ])?
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
                       . entry*
                       . Util.indent . Util.del_str ";;" . eol ] in
      [ keyword_label "case" "@case" . Sep.space
        . store (char+ | ("\"" . char+ . "\""))
        . del /[ \t\n]+/ " " . Util.del_str "in" . eol
        . (empty* . comment* . case_entry)*
        . empty* . comment*
        . keyword "esac" . comment_or_eol ]

  let function (entry:lens) =
    [ Util.indent . label "@function"
      . del /(function[ \t]+)?/ ""
      . store Rx.word . del /[ \t]*\(\)/ "()"
      . eol . Util.del_str "{" . eol
      . entry+
      . Util.indent . Util.del_str "}" . eol ]

  let rec rec_entry =
    let entry = comment | source | kv
              | unset | bare_export | builtin | return | rec_entry in
        cond_if entry
      | loop_for entry
      | loop_select entry
      | loop_while entry
      | loop_until entry
      | case entry
      | function entry

  let lns_norec = empty* . (comment | source | kv | unset | bare_export | builtin | return) *

  let lns = empty* . (comment | source | kv | unset | bare_export | builtin | return | rec_entry) *

  let sc_incl (n:string) = (incl ("/etc/sysconfig/" . n))
  let sc_excl (n:string) = (excl ("/etc/sysconfig/" . n))

  let filter_sysconfig =
      sc_incl "*" .
      sc_excl "bootloader" .
      sc_excl "hw-uuid" .
      sc_excl "hwconf" .
      sc_excl "ip*tables" .
      sc_excl "ip*tables.save" .
      sc_excl "kernel" .
      sc_excl "*.pub" .
      sc_excl "sysstat.ioconf" .
      sc_excl "system-config-firewall" .
      sc_excl "system-config-securitylevel" .
      sc_incl "network/config" .
      sc_incl "network/dhcp" .
      sc_incl "network/dhcp6r" .
      sc_incl "network/dhcp6s" .
      sc_incl "network/ifcfg-*" .
      sc_incl "network/if-down.d/*" .
      sc_incl "network/ifroute-*" .
      sc_incl "network/if-up.d/*" .
      sc_incl "network/providers/*" .
      sc_excl "network-scripts" .
      sc_incl "network-scripts/ifcfg-*" .
      sc_excl "rhn" .
      sc_incl "rhn/allowed-actions/*" .
      sc_excl "rhn/allowed-actions/script" .
      sc_incl "rhn/allowed-actions/script/*" .
      sc_incl "rhn/rhnsd" .
      sc_excl "SuSEfirewall2.d" .
      sc_incl "SuSEfirewall2.d/cobbler" .
      sc_incl "SuSEfirewall2.d/services/*" .
      sc_excl "SuSEfirewall2.d/services/TEMPLATE"

  let filter_default = incl "/etc/default/*"
                     . excl "/etc/default/grub_installdevice*"
                     . excl "/etc/default/whoopsie"
  let filter_misc    = incl "/etc/arno-iptables-firewall/debconf.cfg"
                     . incl "/etc/cron-apt/config"
                     . incl "/etc/environment"
                     . incl "/etc/firewalld/firewalld.conf"
                     . incl "/etc/blkid.conf"
                     . incl "/etc/adduser.conf"
                     . incl "/etc/cowpoke.conf"
                     . incl "/etc/cvs-cron.conf"
                     . incl "/etc/cvs-pserver.conf"
                     . incl "/etc/devscripts.conf"
                     . incl "/etc/lintianrc"
                     . incl "/etc/lsb-release"
                     . incl "/etc/os-release"
                     . incl "/etc/popularity-contest.conf"
                     . incl "/etc/rc.conf"
                     . incl "/etc/rc.conf.local"
                     . incl "/etc/selinux/config"
                     . incl "/etc/ucf.conf"
                     . incl "/etc/locale.conf"
                     . incl "/etc/vconsole.conf"

  let filter = filter_sysconfig
             . filter_default
             . filter_misc
             . Util.stdexcl

  let xfm = transform lns filter

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
