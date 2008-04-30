module Sysconfig =
  autoload xfm

  let sc_incl (n:string) = (incl ("/etc/sysconfig/" . n))

  let filter = Util.stdexcl .
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
      sc_incl "kernel" .
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
      sc_incl "readonly-root" .
      sc_incl "rsyslog" .
      sc_incl "samba" .
      sc_incl "saslauthd" .
      sc_incl "smartmontools" .
      sc_incl "spamassassin" .
      sc_incl "sysstat" .
      sc_incl "system-config-users" .
      sc_incl "vncservers" .
      sc_incl "wpa_supplicant" .
      sc_incl "xend" .
      sc_incl "xendomains"

  let xfm = transform Shellvars.lns filter

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
