(* Dput module for Augeas                     *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)


module Dput =
  autoload xfm

    let setting = IniFile.entry "allow_non-us_software"
                | IniFile.entry "allow_unsigned_uploads"
                | IniFile.entry "check_version"
                | IniFile.entry "default_host_main"
                | IniFile.entry "default_host_non-us"
                | IniFile.entry "fqdn"
                | IniFile.entry "hash"
                | IniFile.entry "incoming"
                | IniFile.entry "login"
                | IniFile.entry "method"
                | IniFile.entry "passive_ftp"
                | IniFile.entry "post_upload_command"
                | IniFile.entry "pre_upload_command"
                | IniFile.entry "progress_indicator"
                | IniFile.entry "run_dinstall"
                | IniFile.entry "run_lintian"
                | IniFile.entry "scp_compress"
		| IniFile.entry "ssh_config_options"

    let record = IniFile.record "target" setting

    let lns = IniFile.lns record

    let filter = (incl "/etc/dput.cf")
        . (incl "~/.dput.cf")
        . Util.stdexcl

    let xfm = transform lns filter
