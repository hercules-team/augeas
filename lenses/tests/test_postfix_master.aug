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

(* fixes bug #69 : accept double quotes in arguments  *)
let conf2 = "# The Cyrus deliver program has changed incompatibly, multiple times.
cyrus     unix  -       n       n       -       -       pipe
  flags=R user=cyrus argv=/usr/sbin/cyrdeliver -e -m \"${extension}\" ${user}
"

test Postfix_Master.lns get conf2 = 
  { "#comment" = "The Cyrus deliver program has changed incompatibly, multiple times." }
  { "cyrus"
    { "type" = "unix" }
    { "private" = "-" }
    { "unpriviliged" = "n" }
    { "chroot" = "n" }
    { "wakeup" = "-" }
    { "limit" = "-" }
    { "command" = "pipe\n  flags=R user=cyrus argv=/usr/sbin/cyrdeliver -e -m \"${extension}\" ${user}" }
  }

