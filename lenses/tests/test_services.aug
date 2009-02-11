(* Tests for the Services module *)

module Test_services =

  let example = "# a comment

tcpmux          1/tcp                           # TCP port service multiplexer
echo            7/udp
discard         9/tcp           sink null
systat          11/tcp          users
# another comment
"

  test Services.lns get example =
    { "#comment" = "a comment" }
    { }
    { "service-name" = "tcpmux"
       { "port"     = "1" }
       { "protocol" = "tcp" }
       { "#comment" = "TCP port service multiplexer" } }
    { "service-name" = "echo"
       { "port"     = "7" }
       { "protocol" = "udp" } }
    { "service-name" = "discard"
       { "port"     = "9" }
       { "protocol" = "tcp" }
       { "alias"    = "sink" }
       { "alias"    = "null" } }
    { "service-name" = "systat"
       { "port"     = "11" }
       { "protocol" = "tcp" }
       { "alias"    = "users" } }
    { "#comment" = "another comment" }
