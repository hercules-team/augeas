module Test_logrotate =

test Logrotate.body get "\n{\n monthly\n}" =
  { "schedule" = "monthly" }

test Logrotate.rule get "/var/log/foo\n{\n monthly\n}\n" =
  { "rule"
    { "file" = "/var/log/foo" }
    { "schedule" = "monthly" } }

test Logrotate.rule get "/var/log/foo /var/log/bar\n{\n monthly\n}\n" =
  { "rule"
    { "file" = "/var/log/foo" }
    { "file" = "/var/log/bar" }
    { "schedule" = "monthly" } }

let conf = "# see man logrotate for details
# rotate log files weekly
weekly

# keep 4 weeks worth of backlogs
rotate 4

# create new (empty) log files after rotating old ones
create

# uncomment this if you want your log files compressed
#compress

tabooext + .old .orig .ignore

# packages drop log rotation information into this directory
include /etc/logrotate.d

# no packages own wtmp, or btmp -- we'll rotate them here
/var/log/wtmp
/var/log/wtmp2
{
    missingok
    monthly
    create 0664 root utmp
    rotate 1
}

/var/log/btmp /var/log/btmp* {
    missingok
    # ftpd doesn't handle SIGHUP properly
    monthly
    create 0664 root utmp
    rotate 1
}
/var/log/vsftpd.log {
    # ftpd doesn't handle SIGHUP properly
    nocompress
    missingok
    notifempty
    rotate 4
    weekly
}

/var/log/apache2/*.log {
        weekly
        missingok
        rotate 52
        compress
        delaycompress
        notifempty
        create 640 root adm
        sharedscripts
        prerotate
                if [ -f /var/run/apache2.pid ]; then
                        /etc/init.d/apache2 restart > /dev/null
                fi
        endscript
}
/var/log/mailman/digest {
    su root list
    monthly
    missingok
    create 0664 list list
    rotate 4
    compress
    delaycompress
        sharedscripts
        postrotate
            [ -f '/var/run/mailman/mailman.pid' ] && /usr/lib/mailman/bin/mailmanctl -q reopen || exit 0
        endscript
}
/var/log/ntp {
    compress
    dateext
    maxage 365
    rotate 99
    size=+2048k
    notifempty
    missingok
    copytruncate
    postrotate
        chmod 644 /var/log/ntp
    endscript
}
"

test Logrotate.lns get conf =
      { "#comment" = "see man logrotate for details" }
      { "#comment" = "rotate log files weekly" }
      { "schedule" = "weekly" }
      {}
      { "#comment" = "keep 4 weeks worth of backlogs" }
      { "rotate"   = "4" }
      {}
      { "#comment" = "create new (empty) log files after rotating old ones" }
      { "create" }
      {}
      { "#comment" = "uncomment this if you want your log files compressed" }
      { "#comment" = "compress" }
      {}
      { "tabooext" = "+"  { ".old" } { ".orig" } { ".ignore" } }
      {}
      { "#comment" = "packages drop log rotation information into this directory" }
      { "include" = "/etc/logrotate.d" }
      {}
      { "#comment" = "no packages own wtmp, or btmp -- we'll rotate them here" }
      { "rule"
           { "file"      = "/var/log/wtmp" }
           { "file"      = "/var/log/wtmp2" }
           { "missingok" = "missingok" }
           { "schedule"  = "monthly" }
           { "create"
                { "mode"  = "0664" }
                { "owner" = "root" }
                { "group" = "utmp" } }
           { "rotate" = "1" } }
      {}
      { "rule"
           { "file"      = "/var/log/btmp" }
	   { "file"      = "/var/log/btmp*" }
           { "missingok" = "missingok" }
	   { "#comment"   = "ftpd doesn't handle SIGHUP properly" }
           { "schedule"  = "monthly" }
           { "create"
                { "mode"  = "0664" }
                { "owner" = "root" }
                { "group" = "utmp" } }
           { "rotate" = "1" } }
      { "rule"
           { "file"      = "/var/log/vsftpd.log" }
           { "#comment"  = "ftpd doesn't handle SIGHUP properly" }
           { "compress"  = "nocompress" }
           { "missingok" = "missingok" }
           { "ifempty"   = "notifempty" }
           { "rotate"    = "4" }
           { "schedule"  = "weekly" } }
      {}
      { "rule"
           { "file"          = "/var/log/apache2/*.log" }
           { "schedule"      = "weekly" }
           { "missingok"     = "missingok" }
           { "rotate"        = "52" }
           { "compress"      = "compress" }
           { "delaycompress" = "delaycompress" }
           { "ifempty"       = "notifempty" }
           { "create"
                { "mode"  = "640"  }
                { "owner" = "root" }
                { "group" = "adm"  } }
           { "sharedscripts" = "sharedscripts" }
           { "prerotate" = "                if [ -f /var/run/apache2.pid ]; then
                        /etc/init.d/apache2 restart > /dev/null
                fi" } }
      { "rule"
           { "file" = "/var/log/mailman/digest" }
           { "su"
               { "owner" = "root" }
               { "group" = "list" } }
           { "schedule"  = "monthly" }
           { "missingok" = "missingok" }
           { "create"
               { "mode"  = "0664" }
               { "owner" = "list" }
               { "group" = "list" } }
           { "rotate"        = "4" }
           { "compress"      = "compress" }
           { "delaycompress" = "delaycompress" }
           { "sharedscripts" = "sharedscripts" }
           { "postrotate"    = "            [ -f '/var/run/mailman/mailman.pid' ] && /usr/lib/mailman/bin/mailmanctl -q reopen || exit 0" } }
      { "rule"
           { "file" = "/var/log/ntp" }
           { "compress" = "compress" }
           { "dateext" = "dateext" }
           { "maxage" = "365" }
           { "rotate" = "99" }
           { "size" = "+2048k" }
           { "ifempty" = "notifempty" }
           { "missingok" = "missingok" }
           { "copytruncate" = "copytruncate" }
           { "postrotate" = "        chmod 644 /var/log/ntp" } }

test Logrotate.lns get "/var/log/file {\n dateext\n}\n" =
    { "rule"
      { "file" = "/var/log/file" }
      { "dateext" = "dateext" } }

  (* Make sure 'minsize 1M' works *)
test Logrotate.lns get "/avr/log/wtmp {\n minsize 1M\n}\n" =
  { "rule"
      { "file" = "/avr/log/wtmp" }
      { "minsize" = "1M" } }

  (* '=' is a legal separator, file names can be indented *)
test Logrotate.lns get " \t /file {\n size=5M\n}\n" =
     { "rule"
         { "file" = "/file" }
         { "size" = "5M" } }

  (* Can leave owner/group off a create statement *)
test Logrotate.lns get "/file {
	create 600\n}\n" =
     { "rule"
         { "file" = "/file" }
         { "create"
             { "mode" = "600" } } }

test Logrotate.lns put "/file {\n	create 600\n}\n" after
    set "/rule/create/owner" "user"
  = "/file {\n	create 600 user\n}\n"

  (* The newline at the end of a script is optional *)
test Logrotate.lns put "/file {\n size=5M\n}\n" after
    set "/rule/prerotate" "\tfoobar"
  =
"/file {
 size=5M
\tprerotate
\tfoobar
\tendscript\n}\n"

test Logrotate.lns put "/file {\n size=5M\n}\n" after
    set "/rule/prerotate" "\tfoobar\n"
  =
"/file {
 size=5M
\tprerotate
\tfoobar\n
\tendscript\n}\n"

(* Bug #101: whitespace at the end of the line *)
test Logrotate.lns get "/file {\n missingok \t\n}\n" =
  { "rule"
    { "file" = "/file" }
    { "missingok" = "missingok" } }

(* Bug #104: file names can be separated by newlines *)
let conf2 = "/var/log/mail.info
/var/log/mail.warn
/var/log/mail.err
{
       weekly
}
"
test Logrotate.lns get conf2 =
  { "rule"
      { "file"      = "/var/log/mail.info" }
      { "file"      = "/var/log/mail.warn" }
      { "file"      = "/var/log/mail.err" }
      { "schedule"  = "weekly" } }
