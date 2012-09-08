module Test_logwatch =

let conf = "# Configuration file for logwatch.
#
#Mailto_host1 = user@example.com

MailFrom = root@example.com
"

test Logwatch.lns get conf =
  { "#comment" = "Configuration file for logwatch." }
  {}
  { "#comment" = "Mailto_host1 = user@example.com" }
  {}
  { "MailFrom" = "root@example.com" }
