module Grub =
  autoload xfm

    (* This only covers the most basic grub directives. Needs to be *)
    (* expanded to cover more (and more esoteric) directives        *)
    (* It is good enough to handle the grub.conf on my Fedora 8 box *)

    let value_to_eol = store /[^= \t][^\n]*/
    let eol = Util.del_str "\n"
    let del_to_eol = del /[^\n]*/ ""
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

    let kw_pres (kw:string) = [ key kw . del_to_eol . eol ]

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
                     | kw_pres "quiet"  (* Seems to be a Ubuntu extension *)
                     | module_lines

    let boot = [ label "title" . title . boot_setting* ]

    let comment = [ del /(#.*|[ \t]*)\n/ "#\n" ]

    let lns = (comment | menu_setting | boot)*
    let xfm = transform lns (incl "/etc/grub.conf")
