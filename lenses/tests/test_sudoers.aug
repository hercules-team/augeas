module Test_sudoers =

   let conf = "
Host_Alias LOCALNET = 192.168.0.0/24, localhost

# User alias specification

# Cmnd alias specification

Cmnd_Alias \
    DEBIAN_TOOLS \
    = \
    /usr/bin/apt-get,\
    /usr/bin/auto-get,   \
    /usr/bin/dpkg, /usr/bin/dselect, /usr/sbin/dpkg-reconfigure \
    : PBUILDER = /usr/sbin/pbuilder

Cmnd_Alias ICAL = /bin/cat /home/rpinson/.kde/share/apps/korganizer/std.ics

Defaults@LOCALNET        !lecture, \
   	tty_tickets,!fqdn

# User privilege specification
root    ALL=(ALL) ALL

# Members of the admin group may gain root privileges
%admin  ALL=(ALL) ALL, NOPASSWD  :	NOSETENV: \
   DEBIAN_TOOLS
%pbuilder       LOCALNET = NOPASSWD: PBUILDER
www-data ALL=(rpinson) NOEXEC: ICAL \
        : \
        localhost = NOPASSWD: 	/usr/bin/test
"

   test Sudoers.lns get conf = 
      {}
      { "Host_Alias"
          { "alias"
	      { "name" = "LOCALNET" }
              { "host" = "192.168.0.0/24" }
              { "host" = "localhost" } } }
      {}
      { "comment" = "User alias specification" }
      {}
      { "comment" = "Cmnd alias specification" }
      {}
      { "Cmnd_Alias"
          { "alias"
              { "name"    = "DEBIAN_TOOLS" }
              { "command" = "/usr/bin/apt-get" }
	      { "command" = "/usr/bin/auto-get" }
	      { "command" = "/usr/bin/dpkg" }
	      { "command" = "/usr/bin/dselect" }
	      { "command" = "/usr/sbin/dpkg-reconfigure" } }
          { "alias"
	      { "name" = "PBUILDER" }
              { "command" = "/usr/sbin/pbuilder" } } }
      {}
      { "Cmnd_Alias"
          { "alias"
	      { "name" = "ICAL" }
              { "command" = "/bin/cat /home/rpinson/.kde/share/apps/korganizer/std.ics" } } }
      {}
      { "Defaults"
          { "type"      = "@LOCALNET" }
	  { "parameter" = "!lecture" }
          { "parameter" = "tty_tickets" }
          { "parameter" = "!fqdn" } }
      {}
      { "comment" = "User privilege specification" }
      { "spec"
          { "user" = "root" }
          { "host_group"
	      { "host" = "ALL" }
	      { "command" = "ALL"
	          { "runas_user"  = "ALL" } } } }
      {}
      { "comment" = "Members of the admin group may gain root privileges" }
      { "spec"
          { "user"    = "%admin" }
	  { "host_group"
	      { "host" = "ALL" }
	      { "command" = "ALL"
	          { "runas_user" = "ALL" } }
	      { "command" = "DEBIAN_TOOLS"
		  { "tag"  = "NOPASSWD" }
		  { "tag"  = "NOSETENV" } } } }
      { "spec"
          { "user"    = "%pbuilder" }
	  { "host_group"
	      { "host" = "LOCALNET" }
	      { "command" = "PBUILDER"
	          { "tag" = "NOPASSWD" } } } }
      { "spec" 
          { "user"    = "www-data" }
	  { "host_group"
	      { "host" = "ALL" }
	      { "command" = "ICAL"
	          { "runas_user" = "rpinson" }
		  { "tag" = "NOEXEC" } } }
	  { "host_group"
	      { "host" = "localhost" }
	      { "command" = "/usr/bin/test"
	          { "tag" = "NOPASSWD" } } } }
