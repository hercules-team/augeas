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
      { "#comment" = "" }
      { "#comment" = "Fichier genere par puppet" }
      { "#comment" = "Environnement: development" }
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
      { "#comment" = "server dns01.echo-net.net" }
      { "restrict"  = "192.168.0.150"
         { "action" = "nomodify" } }
      {}
      { "#comment" = "allow everything from localhost" }
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

  (* Some things needed to process the default ntp.conf on Fedora *)
  test Ntp.lns get
    "server 66.187.233.4  # added by /sbin/dhclient-script\n" =
    { "server" = "66.187.233.4"
      { "#comment" = "# added by /sbin/dhclient-script" } }

  test Ntp.lns get
    "server 0.fedora.pool.ntp.org iburst dynamic\n" =
    { "server" = "0.fedora.pool.ntp.org" { "iburst" } { "dynamic" } }

  test Ntp.lns get
    "restrict 127.0.0.1 \n" =
    { "restrict" = "127.0.0.1" }

  test Ntp.lns get
    "restrict default kod nomodify notrap nopeer noquery\n" =
    { "restrict" = "default"
      { "action" = "kod" }
      { "action" = "nomodify" }
      { "action" = "notrap" }
      { "action" = "nopeer" }
      { "action" = "noquery" } }

  test Ntp.lns put
    "restrict default kod nomodify notrap nopeer noquery\n"
  after
    insb "ipv6" "restrict/action[1]" =
    "restrict -6 default kod nomodify notrap nopeer noquery\n"

  test Ntp.lns get
    "restrict -6 default kod nomodify notrap nopeer noquery\n" =
    { "restrict" = "default"
      { "ipv6" }
      { "action" = "kod" }
      { "action" = "nomodify" }
      { "action" = "notrap" }
      { "action" = "nopeer" }
      { "action" = "noquery" } }

  test Ntp.lns get
    "includefile /etc/ntp/crypto/pw\n" =
    { "includefile" = "/etc/ntp/crypto/pw" }

  test Ntp.lns get "fudge  127.127.1.0 stratum 10\n" =
    { "fudge" = "127.127.1.0" { "stratum" = "10" } }

  test Ntp.lns get "broadcast 192.168.1.255 key 42\n" =
    { "broadcast" = "192.168.1.255" { "key" = "42" } }


  test Ntp.lns get "multicastclient 224.0.1.1\n" =
     { "multicastclient" = "224.0.1.1" }

  test Ntp.lns put "broadcastclient\tnovolley # broadcast\n"
     after rm "/*/novolley" = "broadcastclient # broadcast\n"

  test Ntp.auth_command get "trustedkey 4 8 42\n" =
     { "trustedkey"
         { "key" = "4" }
         { "key" = "8" }
         { "key" = "42" } }

  test Ntp.auth_command get "trustedkey 42\n" =
     { "trustedkey" { "key" = "42" } }

  test Ntp.lns get "broadcastdelay 0.008\n" =
     { "broadcastdelay" = "0.008" }

  test Ntp.lns get "enable auth calibrate\ndisable kernel stats\n" =
     { "enable"
         { "flag" = "auth" }
         { "flag" = "calibrate" } }
     { "disable"
         { "flag" = "kernel" }
         { "flag" = "stats" } }

(* Bug #103: tinker directive *)
test Ntp.tinker get "tinker panic 0 huffpuff 3.14\n" =
  { "tinker"
    { "panic" = "0" }
    { "huffpuff" = "3.14" } }

(* Bug #297: tos directive *)
test Ntp.tos get "tos maxdist 16\n" =
  { "tos"
    { "maxdist" = "16" } }