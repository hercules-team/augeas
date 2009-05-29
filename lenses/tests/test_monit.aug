module Test_monit =

let conf = "# Configuration file for monit.
#
set alert root@localhost
include /my/monit/conf

check process sshd
   start program \"/etc/init.d/ssh start\"
   if failed port 22 protocol ssh then restart

check process httpd with pidfile /usr/local/apache2/logs/httpd.pid
   group www-data
   	 start program \"/usr/local/apache2/bin/apachectl start\"
   stop program \"/usr/local/apache2/bin/apachectl stop\"
"

test Monit.lns get conf =
   { "#comment"  = "Configuration file for monit." }
   {}
   { "set"
     {"alert"    = "root@localhost" } }
   { "include"   = "/my/monit/conf" }
   {}
   { "check"
     { "process" = "sshd" }
     { "start"   = "program \"/etc/init.d/ssh start\"" }
     { "if"      = "failed port 22 protocol ssh then restart" } }
   {}
   { "check"
     { "process" = "httpd with pidfile /usr/local/apache2/logs/httpd.pid" }
     { "group"   = "www-data" }
     { "start"   = "program \"/usr/local/apache2/bin/apachectl start\"" }
     { "stop"    = "program \"/usr/local/apache2/bin/apachectl stop\"" }
}
