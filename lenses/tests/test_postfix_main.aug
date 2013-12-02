module Test_postfix_main =

let conf = "# main.cf
myorigin = /etc/mailname

smtpd_banner = $myhostname ESMTP $mail_name (Ubuntu)
mynetworks = 127.0.0.0/8 [::ffff:127.0.0.0]/104 [::1]/128
relayhost = 
import_environment = 
                 MAIL_CONFIG MAIL_DEBUG MAIL_LOGTAG TZ LANG=C \n  KRB5CCNAME=FILE:${queue_directory}/kerberos/krb5_ccache\n"

test Postfix_Main.lns get conf =
   { "#comment"  = "main.cf" }
   { "myorigin"  = "/etc/mailname" }
   {}
   { "smtpd_banner" = "$myhostname ESMTP $mail_name (Ubuntu)" }
   { "mynetworks" = "127.0.0.0/8 [::ffff:127.0.0.0]/104 [::1]/128" }
   { "relayhost"  }
   { "import_environment" = "MAIL_CONFIG MAIL_DEBUG MAIL_LOGTAG TZ LANG=C \n  KRB5CCNAME=FILE:${queue_directory}/kerberos/krb5_ccache" }

test Postfix_main.lns get "debugger_command =
\t PATH=/bin:/usr/bin:/usr/local/bin:/usr/X11R6/bin
\t ddd $daemon_directory/$process_name $process_id & sleep 5\n"
     =
  { "debugger_command" = "PATH=/bin:/usr/bin:/usr/local/bin:/usr/X11R6/bin
	 ddd $daemon_directory/$process_name $process_id & sleep 5" }
