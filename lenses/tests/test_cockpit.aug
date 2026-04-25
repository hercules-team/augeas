module Test_cockpit =
(************************************************************************
 * Basic Parse
 *************************************************************************)

   let conf = "
[WebService]
Origins = https://somedomain1.com https://somedomain2.com:9090

MaxStartups= 10:30:60
[Log]
Fatal =criticals warnings

[Session]

IdleTimeout=15
"

   test Cockpit.lns get conf =
  {  }
  { "WebService"
    { "Origins" = "https://somedomain1.com" }
    { "Origins" = "https://somedomain2.com:9090" }
    {  }
    { "MaxStartups" = "10:30:60" }
  }
  { "Log"
    { "Fatal" = "criticals" }
    { "Fatal" = "warnings" }
    {  }
  }
  { "Session"
    {  }
    { "IdleTimeout" = "15" }
  }

(************************************************************************
 * Insert simple
 *************************************************************************)
    test Cockpit.lns put conf after
       set "Log/EXAMPLE" "text"
    = "
[WebService]
Origins = https://somedomain1.com https://somedomain2.com:9090

MaxStartups= 10:30:60
[Log]
Fatal =criticals warnings

EXAMPLE=text
[Session]

IdleTimeout=15
"


(************************************************************************
 * Insert Origin makes sense
 *************************************************************************)

    test Cockpit.lns put conf after
       insa "Origins" "WebService/Origins[last()]";
       set "WebService/Origins[last()]" "https://thirdhost.com:8080"
    = "
[WebService]
Origins = https://somedomain1.com https://somedomain2.com:9090 https://thirdhost.com:8080

MaxStartups= 10:30:60
[Log]
Fatal =criticals warnings

[Session]

IdleTimeout=15
"

(************************************************************************
 * Insert Fatal makes sense
 *************************************************************************)
    test Cockpit.lns put conf after
       insa "Fatal" "Log/Fatal[last()]";
       set "Log/Fatal[last()]" "info"
    = "
[WebService]
Origins = https://somedomain1.com https://somedomain2.com:9090

MaxStartups= 10:30:60
[Log]
Fatal =criticals warnings info

[Session]

IdleTimeout=15
"
