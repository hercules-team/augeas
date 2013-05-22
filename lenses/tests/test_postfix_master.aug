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
     { "unprivileged" = "-" }
     { "chroot" = "-" }
     { "wakeup" = "10?" }
     { "limit" = "-" }
     { "command" = "smtpd" } }
   { "maildrop"
     { "type" = "unix" }
     { "private" = "-" }
     { "unprivileged" = "n" }
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
    { "unprivileged" = "n" }
    { "chroot" = "n" }
    { "wakeup" = "-" }
    { "limit" = "-" }
    { "command" = "pipe\n  flags=R user=cyrus argv=/usr/sbin/cyrdeliver -e -m \"${extension}\" ${user}" }
  }

(* accept commas in arguments *)
let conf3 = "# master.cf
submission inet n       -       n       -       -       smtpd
  -o smtpd_client_restrictions=permit_sasl_authenticated,reject
"

test Postfix_Master.lns get conf3 =
   { "#comment" = "master.cf" }
   { "submission"
     { "type" = "inet" }
     { "private" = "n" }
     { "unprivileged" = "-" }
     { "chroot" = "n" }
     { "wakeup" = "-" }
     { "limit" = "-" }
     { "command" = "smtpd\n  -o smtpd_client_restrictions=permit_sasl_authenticated,reject" } }

(* : is allowed *)
let conf4 = "127.0.0.1:10060 inet    n       n       n       -       0       spawn
   user=nobody argv=/usr/sbin/hapolicy -l --default=DEFER
"

test Postfix_Master.lns get conf4 =
  { "127.0.0.1:10060"
    { "type" = "inet" }
    { "private" = "n" }
    { "unprivileged" = "n" }
    { "chroot" = "n" }
    { "wakeup" = "-" }
    { "limit" = "0" }
    { "command" = "spawn
   user=nobody argv=/usr/sbin/hapolicy -l --default=DEFER" }
  }


(* Spaces are allowed after the first word of the command *)
let conf5 = "sympa     unix  -       n       n       -       -       pipe \n	flags=R user=sympa argv=/home/sympa/bin/queue ${recipient}
"

test Postfix_Master.lns get conf5 =
  { "sympa"
    { "type" = "unix" }
    { "private" = "-" }
    { "unprivileged" = "n" }
    { "chroot" = "n" }
    { "wakeup" = "-" }
    { "limit" = "-" }
    { "command" = "pipe \n	flags=R user=sympa argv=/home/sympa/bin/queue ${recipient}" }
  }

(* Arobase is allowed in command *)
let conf6 = "sympafamilypfs  unix  -       n       n       -       -       pipe
	flags=R user=sympa argv=/home/sympa/bin/familyqueue ${user}@domain.net pfs
"
test Postfix_Master.lns get conf6 =
  { "sympafamilypfs"
    { "type" = "unix" }
    { "private" = "-" }
    { "unprivileged" = "n" }
    { "chroot" = "n" }
    { "wakeup" = "-" }
    { "limit" = "-" }
    { "command" = "pipe
	flags=R user=sympa argv=/home/sympa/bin/familyqueue ${user}@domain.net pfs" }
  }

(* Ticket #345 *)
let conf7 = "# master.cf
submission inet n       -       n       -       -       smtpd
  -o mynetworks=127.0.0.1/8,[::1]
"

test Postfix_Master.lns get conf7 =
   { "#comment" = "master.cf" }
   { "submission"
     { "type" = "inet" }
     { "private" = "n" }
     { "unprivileged" = "-" }
     { "chroot" = "n" }
     { "wakeup" = "-" }
     { "limit" = "-" }
     { "command" = "smtpd\n  -o mynetworks=127.0.0.1/8,[::1]" } }
