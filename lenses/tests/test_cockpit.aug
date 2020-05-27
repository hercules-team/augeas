module Test_cockpit =

   let conf = "
[WebService]
Origins = https://somedomain1.com https://somedomain2.com:9090

MaxStartups=10:30:60

[Log]
Fatal=criticals warnings

[Session]
IdleTimeout=15
"

   test Cockpit.lns get conf =
      {}
      { "WebService"
         {}
         { "MaxStartups" = "10:30:60" }
         {}
      }
      { "Log"
         { "Fatal" = "criticals warnings" }
         {}
      }
      { "Session"
         { "IdleTimeout" = "15" }
      }

    test Cockpit.lns put conf after
       set "libvirt/cpus" "2"
    = "
[WebService]
Origins = https://somedomain1.com https://somedomain2.com:9090 https://thirdhost.com:8080

MaxStartups=10:30:60

[Log]
Fatal=criticals warnings

[Session]
IdleTimeout=15
"

