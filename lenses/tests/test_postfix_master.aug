module Test_postfix_master =

let conf = "# master.cf
smtp      inet  n       -       -       10?       -       smtpd
maildrop  unix  -       n       n       -       -       pipe
  flags=DRhu user=vmail argv=/usr/bin/maildrop -d ${recipient}
"

test Postfix_Master.lns get conf =
   { "#comment"  = "master.cf" }
   { "smtp"
     { "type" = "inet" }
     { "private" = "n" }
     { "unpriviliged" = "-" }
     { "chroot" = "-" }
     { "wakeup" = "10?" }
     { "limit" = "-" } 
     { "command" = "smtpd" } }
   { "maildrop"
     { "type" = "unix" }
     { "private" = "-" }
     { "unpriviliged" = "n" }
     { "chroot" = "n" }
     { "wakeup" = "-" }
     { "limit" = "-" } 
     { "command" = "pipe\n  flags=DRhu user=vmail argv=/usr/bin/maildrop -d ${recipient}" } }
