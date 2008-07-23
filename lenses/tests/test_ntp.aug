module Test_ntp =

   let conf = "#
# Fichier genere par puppet
# Environnement: development

server dns01.echo-net.net version 3
server dns02.echo-net.net version 4

driftfile /var/lib/ntp/ntp.drift

restrict default ignore

#server dns01.echo-net.net
restrict 192.168.0.150 nomodify

# allow everything from localhost
restrict 127.0.0.1

logfile /var/log/ntpd
statsdir /var/log/ntpstats/

statistics loopstats peerstats clockstats
filegen loopstats file loopstats type day enable link
filegen peerstats file peerstats type day disable
filegen clockstats file clockstats type day enable nolink
"

   test Ntp.lns get conf = 
      { "comment" = "" }
      { "comment" = "Fichier genere par puppet" }
      { "comment" = "Environnement: development" }
      {}
      { "server" = "dns01.echo-net.net"
         { "version"  = "3" } }
      { "server" = "dns02.echo-net.net"
         { "version"  = "4" } }
      {}
      { "driftfile" = "/var/lib/ntp/ntp.drift" }
      {}
      { "restrict"  = "default" 
         { "action" = "ignore" } }
      {}
      { "comment" = "server dns01.echo-net.net" }
      { "restrict"  = "192.168.0.150"
         { "action" = "nomodify" } }
      {}
      { "comment" = "allow everything from localhost" }
      { "restrict" = "127.0.0.1" }
      {}
      { "logfile"  = "/var/log/ntpd" }
      { "statsdir" = "/var/log/ntpstats/" }
      {}
      { "statistics"
         { "loopstats" }
	 { "peerstats" }
	 { "clockstats" } }
      { "filegen" = "loopstats"
         { "file" = "loopstats" }
	 { "type" = "day" }
	 { "enable" = "enable" }
	 { "link" = "link" } }
      { "filegen" = "peerstats"
         { "file" = "peerstats" }
	 { "type" = "day" }
         { "enable" = "disable" } }
      { "filegen" = "clockstats"
         { "file" = "clockstats" }
	 { "type" = "day" }
         { "enable" = "enable" }
	 { "link" = "nolink" } }
