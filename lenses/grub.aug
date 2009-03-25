module Grub =
  autoload xfm

    (* This only covers the most basic grub directives. Needs to be *)
    (* expanded to cover more (and more esoteric) directives        *)
    (* It is good enough to handle the grub.conf on my Fedora 8 box *)

    let value_to_eol = store /[^= \t\n][^\n]*[^= \t\n]|[^= \t\n]/
    let eol = Util.eol
    let del_to_eol = del /[^ \t\n]*/ ""
    let opt_ws = Util.del_opt_ws ""
    let value_sep (dflt:string) = del /[ \t]*[ \t=][ \t]*/ dflt

    let kw_arg (kw:string) (indent:string) (dflt_sep:string) =
      [ Util.del_opt_ws indent . key kw . value_sep dflt_sep
          . value_to_eol . eol ]

    let kw_boot_arg (kw:string) = kw_arg kw "\t" " "
    let kw_menu_arg (kw:string) = kw_arg kw "" "="
    let password_arg = [ key "password" .
      (Util.del_ws_spc . [ Util.del_str "--md5" . label "md5" ])? .
      Util.del_ws_spc . store (/[^ \t\n]+/ - "--md5") .
      (Util.del_ws_spc . [ label "file" . store /[^ \t\n]+/ ])? .
      eol ]

    let kw_pres (kw:string) = [ opt_ws . key kw . del_to_eol . eol ]

    let color =
      (* Should we nail it down to exactly the color names that *)
      (* grub supports ? *)
      let color_name = store /[A-Za-z-]+/ in
      let color_spec =
        [ label "foreground" . color_name] .
        Util.del_str "/" .
        [ label "background" . color_name ] in
      [ opt_ws . key "color" .
        Util.del_ws_spc . [ label "normal" . color_spec ] .
        (Util.del_ws_spc . [ label "highlight" . color_spec ])? .
        eol ]

    let menu_setting = kw_menu_arg "default"
                     | kw_menu_arg "fallback"
                     | kw_pres "hiddenmenu"
                     | kw_menu_arg "timeout"
                     | kw_menu_arg "splashimage"
                     | kw_menu_arg "serial"
                     | kw_menu_arg "terminal"
                     | password_arg
                     | color

    let title = del /title[ \t]+/ "title " . value_to_eol . eol

    let module_lines = [ label "modules" .
                            Util.del_ws "\t" .
                             Util.del_str "module" . Util.del_ws_spc
                             . value_to_eol . eol ]

    let boot_setting = kw_boot_arg "root"
                     | kw_boot_arg "kernel"
                     | kw_boot_arg "initrd"
                     | kw_boot_arg "rootnoverify"
                     | kw_boot_arg "chainloader"
                     | kw_boot_arg "uuid"
                     | kw_pres "quiet"  (* Seems to be a Ubuntu extension *)
                     | kw_pres "savedefault"
                     | module_lines

    let boot = [ label "title" . title . boot_setting* ]

    let comment_re = /([^ \t\n].*[^ \t\n]|[^ \t\n])/
                       - "# ## Start Default Options ##"
                       - "# ## End Default Options ##"

    let comment    =
        [ Util.indent . label "#comment" . del /#[ \t]*/ "# "
            . store comment_re . eol ]
    let empty   = Util.empty


    let debian_header  = "## ## Start Default Options ##\n"
    let debian_footer  = "## ## End Default Options ##\n"
    let debian_comment_re = /([^ \t\n].*[^ \t\n]|[^ \t\n])/
                            - "## End Default Options ##"
    let debian_comment =
        [ Util.indent . label "#comment" . del /##[ \t]*/ "## "
            . store debian_comment_re . eol ]
    let debian_value_sep = del /[ \t]*=/ "="
    let debian_kw_arg (kw:regexp) =
      [ Util.del_opt_ws "" . key kw . debian_value_sep
          . value_to_eol? . eol ]

    let debian_setting_re = "kopt"
                          | "groot"
                          | "alternative"
                          | "lockalternative"
                          | "defoptions"
                          | "lockold"
                          | "xenhopt"
                          | "xenkopt"
                          | "altoptions"
                          | "howmany"
                          | "memtest86"
                          | "updatedefaultentry"
                          | "savedefault"

    let debian_setting = debian_kw_arg debian_setting_re

    let debian_entry   = del "#" "#" . debian_setting
    let debian         = [ label "debian"
                    . del debian_header debian_header
                    . (debian_comment|empty|debian_entry)*
                    . del debian_footer debian_footer ]

    let lns = (comment | empty | menu_setting | boot | debian)*
    let xfm = transform lns (incl "/etc/grub.conf")
