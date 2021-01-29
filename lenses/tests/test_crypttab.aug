module Test_crypttab =

  let simple = "sda1_crypt\t /dev/sda1\t    /dev/random \t      swap\n"

  let simple_tree =
    { "1"
        { "target" = "sda1_crypt" }
        { "device" = "/dev/sda1" }
        { "password" = "/dev/random" }
        { "opt" = "swap" } }

  let trailing_ws = "sda1_crypt\t /dev/sda1\t    /dev/random \t      swap\t\n"

  let no_opts = "sda1_crypt\t /dev/sda1\t    /etc/key\n"

  let no_opts_tree =
    { "1"
        { "target" = "sda1_crypt" }
        { "device" = "/dev/sda1" }
        { "password" = "/etc/key" } }

  let no_password = "sda1_crypt\t /dev/sda1\n"

  let no_password_tree =
    { "1"
        { "target" = "sda1_crypt" }
        { "device" = "/dev/sda1" } }

  let multi_opts = "sda1_crypt\t /dev/sda1\t    /etc/key \t      cipher=aes-cbc-essiv:sha256,verify\n"

  let multi_opts_tree =
    { "1"
        { "target" = "sda1_crypt" }
        { "device" = "/dev/sda1" }
        { "password" = "/etc/key" }
        { "opt" = "cipher"
            { "value" = "aes-cbc-essiv:sha256" } }
        { "opt" = "verify" } }

  let uuid = "sda3_crypt UUID=5b8b6e72-acf9-43bc-bd2d-8dbcaee82f99 none luks,keyscript=/usr/share/yubikey-luks/ykluks-keyscript,discard\n"

  let uuid_tree =
    { "1"
        { "target" = "sda3_crypt" }
        { "device" = "UUID=5b8b6e72-acf9-43bc-bd2d-8dbcaee82f99" }
        { "password" = "none" }
        { "opt" = "luks" }
        { "opt" = "keyscript"
            { "value" = "/usr/share/yubikey-luks/ykluks-keyscript" } }
        { "opt" = "discard" } }

  test Crypttab.lns get simple = simple_tree

  test Crypttab.lns get trailing_ws = simple_tree

  test Crypttab.lns get no_opts = no_opts_tree

  test Crypttab.lns get no_password = no_password_tree

  test Crypttab.lns get multi_opts = multi_opts_tree

  test Crypttab.lns get uuid = uuid_tree
