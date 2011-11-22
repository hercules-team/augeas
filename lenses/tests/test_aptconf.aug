module Test_aptconf =

 (* Test multiline C-style comments *)
 let comment_multiline = "/* This is a long
/* multiline
comment
*/
"

  test AptConf.comment get comment_multiline =
     { "#mcomment"
        { "1" = "This is a long" }
        { "2" = "/* multiline" }
        { "3" = "comment" } }


  (* Test empty multiline C-style comments *)
  let comment_multiline_empty = "/* */\n"

  test AptConf.empty get comment_multiline_empty = { }


  (* Test a simple entry *)
  let simple_entry = "APT::Clean-Installed \"true\";\n"

  test AptConf.entry get simple_entry =
     { "APT" { "Clean-Installed" = "true" } }

  (* Test simple recursivity *)
  let simple_recursion = "APT { Clean-Installed \"true\"; };\n"

  test AptConf.entry get simple_recursion =
     { "APT" { "Clean-Installed" = "true" } }

  (* Test simple recursivity with several entries *)
  let simple_recursion_multi =
    "APT {
         Clean-Installed \"true\";
         Get::Assume-Yes \"true\";
     }\n"

  test AptConf.entry get simple_recursion_multi =
     { "APT"
        { "Clean-Installed" = "true" }
        { "Get" { "Assume-Yes" = "true" } } }

  (* Test multiple recursivity *)
  let multiple_recursion =
    "APT { Get { Assume-Yes \"true\"; } };\n"

  test AptConf.entry get multiple_recursion =
     { "APT" { "Get" { "Assume-Yes" = "true" } } }

  (* Test simple list *)
  let simple_list = "DPKG::options { \"--force-confold\"; }\n"

  test AptConf.entry get simple_list =
     { "DPKG" { "options" { "@elem" = "--force-confold" } } }


  (* Test list elements with spaces *)
  let list_spaces = "Unattended-Upgrade::Allowed-Origins {
       \"Ubuntu lucid-security\"; };\n"

  test AptConf.entry get list_spaces =
     { "Unattended-Upgrade" { "Allowed-Origins"
       { "@elem" = "Ubuntu lucid-security" } } }

  (* Test recursive list *)
  let recursive_list =
    "DPKG {
         options {
             \"--force-confold\";
             \"--nocheck\";
         } };\n"

  test AptConf.entry get recursive_list =
     { "DPKG"
        { "options"
           { "@elem" = "--force-confold" }
           { "@elem" = "--nocheck" } } }

  (* Test empty group *)
  let empty_group =
   "APT\n{\n};\n"

  test AptConf.entry get empty_group = { "APT" }

  (* Test #include *)
  let include = "  #include /path/to/file\n"

  test AptConf.include get include =
     { "#include" = "/path/to/file" }

  (* Test #clear *)
  let clear = "#clear Dpkg::options Apt::Get::Assume-Yes\n"

  test AptConf.clear get clear =
     { "#clear"
        { "name" = "Dpkg::options" }
        { "name" = "Apt::Get::Assume-Yes" } }


  (* Test put simple value *)
  test AptConf.entry put "APT::Clean-Installed \"true\";\n"
     after set "/APT/Clean-Installed" "false" =
     "APT {\nClean-Installed \"false\";\n};\n"

  (* Test rm everything *)
  test AptConf.entry put "APT { Clean-Installed \"true\"; }\n"
     after rm "/APT" = ""

  (* Test rm on recursive value *)
  test AptConf.entry put "APT { Clean-Installed \"true\"; }\n"
     after rm "/APT/Clean-Installed" = "APT { }\n"

  (* Test put recursive value *)
  test AptConf.entry put "APT { Clean-Installed \"true\"; }\n"
     after set "/APT/Clean-Installed" "false" =
     "APT { Clean-Installed \"false\"; }\n"

  (* Test multiple lens *)
  let multiple_entries =
     "APT { Clean-Installed \"true\"; }\n
      APT::Clean-Installed \"true\";\n"

  test AptConf.lns get multiple_entries =
     { "APT" { "Clean-Installed" = "true" } }
     {}
     { "APT" { "Clean-Installed" = "true" } }

  (* Test with full lens *)
  test AptConf.lns put "APT { Clean-Installed \"true\"; }\n"
     after set "/APT/Clean-Installed" "false" =
     "APT { Clean-Installed \"false\"; }\n"

  (* Test single commented entry *)
  let commented_entry =
      "Unattended-Upgrade::Allowed-Origins {
       \"Ubuntu lucid-security\";
//      \"Ubuntu lucid-updates\";
       };\n"

  test AptConf.lns get commented_entry =
     { "Unattended-Upgrade" { "Allowed-Origins"
       { "@elem" = "Ubuntu lucid-security" }
       { "#comment" = "\"Ubuntu lucid-updates\";" } } }

  (* Test multiple commented entries *)
  let commented_entries =
      "// List of packages to not update
Unattended-Upgrade::Package-Blacklist {
//      \"vim\";
//      \"libc6\";
//      \"libc6-dev\";
//      \"libc6-i686\"
};
"

  test AptConf.lns get commented_entries =
     { "#comment" = "List of packages to not update" }
     { "Unattended-Upgrade" { "Package-Blacklist"
       { "#comment" = "\"vim\";" }
       { "#comment" = "\"libc6\";" }
       { "#comment" = "\"libc6-dev\";" }
       { "#comment" = "\"libc6-i686\"" }
     } }

  (* Test complex elem *)
  let complex_elem = "DPkg::Post-Invoke {\"if [ -d /var/lib/update-notifier ]; then touch /var/lib/update-notifier/dpkg-run-stamp; fi; if [ -e /var/lib/update-notifier/updates-available ]; then echo > /var/lib/update-notifier/updates-available; fi \"};\n"

  test AptConf.lns get complex_elem =
     { "DPkg" { "Post-Invoke"
       { "@elem" = "if [ -d /var/lib/update-notifier ]; then touch /var/lib/update-notifier/dpkg-run-stamp; fi; if [ -e /var/lib/update-notifier/updates-available ]; then echo > /var/lib/update-notifier/updates-available; fi " } } }
