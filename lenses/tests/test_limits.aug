module Test_limits =

  let conf = "@audio - rtprio 99
ftp hard nproc /ftp
* soft core 0
"

  test Limits.lns get conf =
    { "domain" = "@audio"
      { "type"  = "-" }
      { "item"  = "rtprio" }
      { "value" = "99" } }
    { "domain" = "ftp"
      { "type" = "hard" }
      { "item" = "nproc" }
      { "value" = "/ftp" } }
    { "domain" = "*"
      { "type" = "soft" }
      { "item" = "core" }
      { "value" = "0" } }

  test Limits.lns put conf after
    insa "domain" "domain[last()]" ;
    set "domain[last()]" "*" ;
    set "domain[last()]/type" "-" ;
    set "domain[last()]/item" "nofile" ;
    set "domain[last()]/value" "4096"
  = "@audio - rtprio 99
ftp hard nproc /ftp
* soft core 0
* - nofile 4096\n"

  test Limits.lns get "* soft core 0 # clever comment\n" =
    { "domain" = "*"
      { "type" = "soft" }
      { "item" = "core" }
      { "value" = "0" }
      { "#comment" = "clever comment" } }
