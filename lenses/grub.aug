module Grub =
  autoload xfm

    (* This only covers the most basic grub directives. Needs to be *)
    (* expanded to cover more (and more esoteric) directives        *)
    (* It is good enough to handle the grub.conf on my Fedora 8 box *)

    let value_to_eol = store /[^= \t\n][^\n]*[^= \t\n]|[^= \t\n]/
    let eol = Util.eol
    let del_to_eol = del /[^ \t\n]*/ ""
    let spc = Util.del_ws_spc
    let opt_ws = Util.del_opt_ws ""
    let dels (s:string) = Util.del_str s
    let eq = dels "="
    let switch (n:regexp) = dels "--" . key n
    let switch_arg (n:regexp) = switch n . eq . store Rx.no_spaces
    let value_sep (dflt:string) = del /[ \t]*[ \t=][ \t]*/ dflt

    let comment_re = /([^ \t\n].*[^ \t\n]|[^ \t\n])/
                       - /# ## (Start|End) Default Options ##/

    let comment    =
        [ Util.indent . label "#comment" . del /#[ \t]*/ "# "
            . store comment_re . eol ]
    let empty   = Util.empty

    let command (kw:string) (indent:string) =
      Util.del_opt_ws indent . key kw

    let kw_arg (kw:string) (indent:string) (dflt_sep:string) =
      [ command kw indent . value_sep dflt_sep . value_to_eol . eol ]

    let kw_boot_arg (kw:string) = kw_arg kw "\t" " "
    let kw_menu_arg (kw:string) = kw_arg kw "" " "
    let password_arg = [ key "password" .
      (spc . [ switch "md5" ])? .
      spc . store (/[^ \t\n]+/ - "--md5") .
      (spc . [ label "file" . store /[^ \t\n]+/ ])? .
      eol ]

    let kw_pres (kw:string) = [ opt_ws . key kw . del_to_eol . eol ]

    let color =
      (* Should we nail it down to exactly the color names that *)
      (* grub supports ? *)
      let color_name = store /[A-Za-z-]+/ in
      let color_spec =
        [ label "foreground" . color_name] .
        dels "/" .
        [ label "background" . color_name ] in
      [ opt_ws . key "color" .
        spc . [ label "normal" . color_spec ] .
        (spc . [ label "highlight" . color_spec ])? .
        eol ]

    let serial =
      [ command "serial" "" .
        [ spc . switch_arg /unit|port|speed|word|parity|stop|device/ ]* .
        eol ]

    let terminal =
      [ command "terminal" "" .
          ([ spc . switch /dumb|no-echo|no-edit|silent/ ]
          |[ spc . switch_arg /timeout|lines/ ])* .
          [ spc . key /console|serial|hercules/ ]* . eol ]

    let menu_setting = kw_menu_arg "default"
                     | kw_menu_arg "fallback"
                     | kw_pres "hiddenmenu"
                     | kw_menu_arg "timeout"
                     | kw_menu_arg "splashimage"
                     | kw_menu_arg "gfxmenu"
                     | serial
                     | terminal
                     | password_arg
                     | color

    let title = del /title[ \t=]+/ "title " . value_to_eol . eol

    (* Parse the file name and args on a kernel or module line *)
    let kernel_args =
      let arg = Rx.word - /type|no-mem-option/  in
      store /\/[^ \t\n]*/ .
            (spc . [ key arg . (eq. store /([^ \t\n])*/)?])* . eol

    let module_line =
      [ command "module" "\t" . spc . kernel_args ]

    let map_line =
      [ command "map" "\t" . spc .
           [ label "from" . store /[()A-za-z0-9]+/ ] . spc .
           [ label "to" . store /[()A-za-z0-9]+/ ] . eol ]

    let kernel =
        [ command "kernel" "\t" .
          (spc .
             ([switch "type" . eq . store /[a-z]+/]
             |[switch "no-mem-option"]))* .
          spc . kernel_args ]

    let chainloader =
      [ command "chainloader" "\t" .
          [ spc . switch "force" ]? . spc . store Rx.no_spaces . eol ]

    let savedefault =
      [ command "savedefault" "\t" . (spc . store Rx.integer)? . eol ]

    let boot_setting = kw_boot_arg "root"
                     | kernel
                     | kw_boot_arg "initrd"
                     | kw_boot_arg "rootnoverify"
                     | chainloader
                     | kw_boot_arg "uuid"
                     | kw_pres "quiet"  (* Seems to be a Ubuntu extension *)
                     | savedefault
                     | module_line
                     | map_line

    let boot =
      let line = ((boot_setting|comment)* . boot_setting)? in
      [ label "title" . title . line ]

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
                          | "indomU"

    let debian_setting = debian_kw_arg debian_setting_re

    let debian_entry   = del "#" "#" . debian_setting
    let debian         = [ label "debian"
                    . del debian_header debian_header
                    . (debian_comment|empty|debian_entry)*
                    . del debian_footer debian_footer ]

    let lns = (comment | empty | menu_setting | boot | debian)*
    let xfm = transform lns (incl "/boot/grub/menu.lst"
                           . incl "/etc/grub.conf")
