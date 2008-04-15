module Grub =
  autoload xfm

    (* This only covers the most basic grub directives. Needs to be *)
    (* expanded to cover more (and more esoteric) directives        *)
    (* It is good enough to handle the grub.conf on my Fedora 8 box *)

    let value_to_eol = store /[^= \t][^\n]*/
    let eol = Util.del_str "\n"
    let del_to_eol = del /[^\n]*/ ""
    let value_sep (dflt:string) = del /[ \t]*[ \t=][ \t]*/ dflt

    let kw_arg (kw:string) (indent:string) (dflt_sep:string) = 
      [ Util.del_opt_ws indent . key kw . value_sep dflt_sep 
          . value_to_eol . eol ]

    let kw_boot_arg (kw:string) = kw_arg kw "\t" " "
    let kw_menu_arg (kw:string) = kw_arg kw "" "="

    let kw_pres (kw:string) = [ key kw . del_to_eol . eol ]

    let menu_setting = kw_menu_arg "default"
                     | kw_menu_arg "fallback"
                     | kw_pres "hiddenmenu"
                     | kw_menu_arg "timeout"
                     | kw_menu_arg "splashimage"

    let title = del /title[ \t]+/ "title " . value_to_eol . eol

    let module_lines = [ label "modules" . 
                            Util.del_ws "\t" .
                             Util.del_str "module" . Util.del_ws_spc
                             . value_to_eol . eol ]

    let boot_setting = kw_boot_arg "root"
                     | kw_boot_arg "kernel"
                     | kw_boot_arg "initrd"
                     | module_lines

    let boot = [ label "title" . title . boot_setting* ]

    let comment = [ del /#.*\n/ "# " ]

    let lns = (comment | menu_setting | boot)*
    let xfm = transform lns (incl "/etc/grub.conf")
