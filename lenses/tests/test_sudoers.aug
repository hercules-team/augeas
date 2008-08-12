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

Defaults:buildd env_keep+=\"APT_CONFIG DEBIAN_FRONTEND SHELL\"

# User privilege specification
root    ALL=(ALL) ALL

# Members of the admin group may gain root privileges
%admin  ALL=(ALL) ALL, NOPASSWD  :	NOSETENV: \
   DEBIAN_TOOLS
%pbuilder       LOCALNET = NOPASSWD: PBUILDER
www-data +biglab=(rpinson)NOEXEC: ICAL \
        : \
        localhost = NOPASSWD: 	/usr/bin/test

	+secretaries           ALPHA = /usr/bin/su [!-]*, !/usr/bin/su *root*
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
      { "Defaults"
          { "type"      = ":buildd" }
	  { "parameter" = "env_keep+=\"APT_CONFIG DEBIAN_FRONTEND SHELL\"" } }
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
	      { "host" = "+biglab" }
	      { "command" = "ICAL"
	          { "runas_user" = "rpinson" }
		  { "tag" = "NOEXEC" } } }
	  { "host_group"
	      { "host" = "localhost" }
	      { "command" = "/usr/bin/test"
	          { "tag" = "NOPASSWD" } } } }
      {}
      { "spec"
          { "user"    = "+secretaries" }
	  { "host_group"
	      { "host" = "ALPHA" }
	      { "command" = "/usr/bin/su [!-]*" }
	      { "command" = "!/usr/bin/su *root*" } } }
