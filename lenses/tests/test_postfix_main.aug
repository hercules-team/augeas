module Test_postfix_main =

let conf = "# main.cf
myorigin = /etc/mailname

smtpd_banner = $myhostname ESMTP $mail_name (Ubuntu)
mynetworks = 127.0.0.0/8 [::ffff:127.0.0.0]/104 [::1]/128
relayhost = 
"

test Postfix_Main.lns get conf =
   { "#comment"  = "main.cf" }
   { "myorigin"  = "/etc/mailname" }
   {}
   { "smtpd_banner" = "$myhostname ESMTP $mail_name (Ubuntu)" }
   { "mynetworks" = "127.0.0.0/8 [::ffff:127.0.0.0]/104 [::1]/128" }
   { "relayhost"  }
