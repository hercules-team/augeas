module Test_nsswitch =

  let conf = "# Sample nsswitch.conf
passwd:         compat

hosts:          files mdns4_minimal [NOTFOUND=return] dns mdns4
networks:       nis [!UNAVAIL=return success=continue] files
protocols:      db files
netgroup:       nis
bootparams: nisplus [NOTFOUND=return] files
sudoers:        files ldap
"

test Nsswitch.lns get conf =
   { "#comment" = "Sample nsswitch.conf" }
   { "database" = "passwd"
      { "service" = "compat" } }
   {}
   { "database" = "hosts"
      { "service" = "files" }
      { "service" = "mdns4_minimal" }
      { "reaction"
           { "status" = "NOTFOUND"
               { "action" = "return" } } }
      { "service" = "dns" }
      { "service" = "mdns4" } }
   { "database" = "networks"
      { "service" = "nis" }
      { "reaction"
           { "status" = "UNAVAIL"
               { "negate" }
               { "action" = "return" } }
           { "status" = "success"
               { "action" = "continue" } } }
      { "service" = "files" } }
   { "database" = "protocols"
      { "service" = "db" }
      { "service" = "files" } }
   { "database" = "netgroup"
      { "service" = "nis" } }
   { "database" = "bootparams"
      { "service" = "nisplus" }
      { "reaction"
           { "status" = "NOTFOUND"
               { "action" = "return" } } }
      { "service" = "files" } }
   { "database" = "sudoers"
      { "service" = "files" }
      { "service" = "ldap" } }

