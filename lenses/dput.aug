(* Dput module for Augeas                     *)
(* Author: Raphael Pinson <raphink@gmail.com> *)
(*                                            *)


module Dput =
  autoload xfm

    let setting = "allow_non-us_software"
                | "allow_unsigned_uploads"
                | "check_version"
                | "default_host_main"
                | "default_host_non-us"
                | "fqdn"
                | "hash"
                | "incoming"
                | "login"
                | "method"
                | "passive_ftp"
                | "post_upload_command"
                | "pre_upload_command"
                | "progress_indicator"
                | "run_dinstall"
                | "run_lintian"
                | "scp_compress"
		| "ssh_config_options"
	
    let entry = IniFile.entry setting

    let record = IniFile.record "target" entry

    let lns = IniFile.lns record

    let filter = (incl "/etc/dput.cf")
        . (incl "~/.dput.cf")
        . Util.stdexcl

    let xfm = transform lns filter
