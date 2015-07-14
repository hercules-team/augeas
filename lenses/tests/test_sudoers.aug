(* Module: Test_sudoers *)
module Test_sudoers =

let test_user = [ label "user" . Sudoers.sto_to_com_user . Util.eol ]*

(* Test: test_user *)
test test_user get "root
@pbuilder
+secre-taries
@my\ admin\ group
EXAMPLE\\\\cslack
MY\ EX-AMPLE\ 9\\\\cslack\ group
" =
  { "user" = "root" }
  { "user" = "@pbuilder" }
  { "user" = "+secre-taries" }
  { "user" = "@my\\ admin\\ group" }
  { "user" = "EXAMPLE\\\\cslack" }
  { "user" = "MY\\ EX-AMPLE\\ 9\\\\cslack\\ group" }

let conf = "
  Host_Alias LOCALNET = 192.168.0.0/24, localhost

   # User alias specification

User_Alias EXAMPLE_ADMINS = cslack, EXAMPLE\\\\cslack,\
          EXAMPLE\\\\jmalstrom

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
   \t\t tty_tickets,!fqdn, !!env_reset

Defaults   !visiblepw

Defaults:buildd env_keep+=\"APT_CONFIG DEBIAN_FRONTEND SHELL\"
Defaults!PBUILDER env_keep+=\"HOME ARCH DIST DISTRIBUTION PDEBUILD_PBUILDER\"

# User privilege specification
root    ALL=(ALL) ALL
root    ALL=(: ALL) ALL
root    ALL=(ALL :ALL) ALL

# Members of the admin group may gain root privileges
%admin  ALL=(ALL) ALL, NOPASSWD  :	NOSETENV: \
   DEBIAN_TOOLS
%pbuilder       LOCALNET = NOPASSWD: PBUILDER
www-data +biglab=(rpinson)NOEXEC: ICAL \
        : \
        localhost = NOPASSWD: 	/usr/bin/test

	+secretaries           ALPHA = /usr/bin/su [!-]*, !/usr/bin/su *root*

@my\ admin\ group ALL=(root) NOPASSWD: /usr/bin/python /usr/local/sbin/filterlog -iu\\=www /var/log/something.log
#includedir /etc/sudoers.d
#include /etc/sudoers.d
"

   test Sudoers.lns get conf =
      {}
      { "Host_Alias"
          { "alias"
	      { "name" = "LOCALNET" }
              { "host" = "192.168.0.0/24" }
              { "host" = "localhost" } } }
      {}
      { "#comment" = "User alias specification" }
      {}
      { "User_Alias"
          { "alias"
              { "name" = "EXAMPLE_ADMINS" }
              { "user" = "cslack" }
              { "user" = "EXAMPLE\\\\cslack" }
              { "user" = "EXAMPLE\\\\jmalstrom" } } }
      {}
      { "#comment" = "Cmnd alias specification" }
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
	      { "lecture" { "negate" } }
          { "tty_tickets" }
          { "fqdn" { "negate" } }
          { "env_reset" } }
      {}
      { "Defaults"
          { "visiblepw" { "negate" } } }
      {}
      { "Defaults"
          { "type"      = ":buildd" }
	      { "env_keep"
              { "append" }
              { "var" = "APT_CONFIG" }
              { "var" = "DEBIAN_FRONTEND" }
              { "var" = "SHELL" } } }
      { "Defaults"
          { "type" = "!PBUILDER" }
          { "env_keep"
              { "append" }
              { "var" = "HOME" }
              { "var" = "ARCH" }
              { "var" = "DIST" }
              { "var" = "DISTRIBUTION" }
              { "var" = "PDEBUILD_PBUILDER" } } }
      {}
      { "#comment" = "User privilege specification" }
      { "spec"
          { "user" = "root" }
          { "host_group"
	      { "host" = "ALL" }
	      { "command" = "ALL"
	          { "runas_user"  = "ALL" } } } }
      { "spec"
          { "user" = "root" }
          { "host_group"
	      { "host" = "ALL" }
	      { "command" = "ALL"
            { "runas_group" = "ALL" } } } }
      { "spec"
          { "user" = "root" }
          { "host_group"
	      { "host" = "ALL" }
	      { "command" = "ALL"
	          { "runas_user"  = "ALL" }
            { "runas_group" = "ALL" } } } }
      {}
      { "#comment" = "Members of the admin group may gain root privileges" }
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
	      { "command" = "/usr/bin/su *root*" 
                  { "negate" } } } }
      {}
      { "spec"
          { "user"    = "@my\ admin\ group" }
          { "host_group"
              { "host" = "ALL" }
              { "command" = "/usr/bin/python /usr/local/sbin/filterlog -iu\\=www /var/log/something.log"
                  { "runas_user" = "root" }
                  { "tag" = "NOPASSWD" }
              }
          }
      }
      { "#includedir" = "/etc/sudoers.d" }
      { "#include" = "/etc/sudoers.d" }

test Sudoers.parameter_integer_bool
    put "umask = 022"
    after set "/umask/negate" ""  = "!umask"

test Sudoers.parameter_integer_bool
    put "!!!!!umask"
    after rm "/umask/negate"; set "/umask" "022" = "!!!!umask = 022"

test Sudoers.parameter_integer_bool put "!!!!umask = 022" after
    set "/umask/negate" "" = "!!!!!umask"

test Sudoers.parameter_integer_bool get "!!!umask = 022" = *

(* BZ 566134 *)

let s = "Defaults    secure_path = /sbin:/bin:/usr/sbin:/usr/bin\n"
test Sudoers.lns get s =
  { "Defaults"
    { "secure_path" = "/sbin:/bin:/usr/sbin:/usr/bin" } }

(* Ticket #206, comments at end of lines *)
let commenteol = "#
Defaults targetpw    # ask for
Host_Alias LOCALNET = 192.168.0.0/24   # foo eol
root    ALL=(ALL) ALL                  # all root\n"
test Sudoers.lns get commenteol =
  {}
  { "Defaults"
    { "targetpw" }
    { "#comment" = "ask for" } }
  { "Host_Alias"
      { "alias"
    { "name" = "LOCALNET" }
          { "host" = "192.168.0.0/24" } }
    { "#comment" = "foo eol" } }
  { "spec"
      { "user" = "root" }
      { "host_group"
    { "host" = "ALL" }
    { "command" = "ALL"
        { "runas_user"  = "ALL" } } }
    { "#comment" = "all root" } }

(* Allow = in commands *)
test Sudoers.spec get "root ALL= /usr/bin/mylvmbackup --configfile=/etc/mylvbackup_amanda.conf\n" =
  { "spec"
    { "user" = "root" }
    { "host_group"
      { "host" = "ALL" }
      { "command" = "/usr/bin/mylvmbackup --configfile=/etc/mylvbackup_amanda.conf" } } }

(* Allow commands without full path
   -- if they begin with a lowcase letter *)
test Sudoers.spec get "root ALL= sudoedit /etc/passwd\n" =
  { "spec"
    { "user" = "root" }
    { "host_group"
      { "host" = "ALL" }
      { "command" = "sudoedit /etc/passwd" } } }

(* Ticket #263, quoted values in defaults line *)
let defaults_spaces = "Defaults       passprompt=\"Your SecurID Passcode: \"\n"
test Sudoers.lns get defaults_spaces =
  { "Defaults"
    { "passprompt" = "\"Your SecurID Passcode: \"" }
  }

(* Ticket #263, quoted values in defaults line (string/bool parameters) *)
let defaults_spaces_strbool = "Defaults      mailfrom=\"root@example.com\"\n"
test Sudoers.lns get defaults_spaces_strbool =
  { "Defaults"
    { "mailfrom" = "\"root@example.com\"" }
  }

(* Test: Sudoers.spec
     Spec users can be aliases *)
test Sudoers.spec get "APACHE_ADMIN ALL= ALL\n" =
  { "spec"
    { "user" = "APACHE_ADMIN" }
    { "host_group"
      { "host" = "ALL" }
      { "command" = "ALL" } } }

(* Test: Sudoers.spec
     Ticket #337: allow period in user names *)
test Sudoers.spec get "user.one somehost = ALL\n" =
  { "spec"
    { "user" = "user.one" }
    { "host_group"
      { "host" = "somehost" }
      { "command" = "ALL" }
    }
  }

(* Test: Sudoers.spec
     Ticket #370: allow underscore in group names *)
test Sudoers.spec get "%sudo_users ALL=(ALL) ALL\n" =
  { "spec"
    { "user" = "%sudo_users" }
    { "host_group"
      { "host" = "ALL" }
      { "command" = "ALL"
        { "runas_user" = "ALL" } }
    }
  }

(* Test: Sudoers.spec
     Ticket #376: allow uppercase characters in user names *)
test Sudoers.spec get "%GrOup ALL = (ALL) ALL\n" =
  { "spec"
    { "user" = "%GrOup" }
    { "host_group"
      { "host" = "ALL" }
      { "command" = "ALL"
        { "runas_user" = "ALL" } }
    }
  }

(* Test: Sudoers.spec
     allow + in user-/groupnames *)
test Sudoers.spec get "group+user somehost = ALL\n" =
  { "spec"
    { "user" = "group+user" }
    { "host_group"
      { "host" = "somehost" }
      { "command" = "ALL" }
    }
  }

(* Test: Sudoers.spec
     GH #262:  Sudoers lens doesn't support `!` for command aliases *)
test Sudoers.spec get "%opssudoers ALL=(ALL) ALL, !!!BANNED\n" = 
  { "spec"
    { "user" = "%opssudoers" }
    { "host_group"
      { "host" = "ALL" }
      { "command" = "ALL"
        { "runas_user" = "ALL" } }
      { "command" = "BANNED"
        { "negate" } }
    }
  }

(* Test: Sudoers.spec
     Handle multiple `!` properly in commands *)
test Sudoers.spec get "%opssudoers ALL=(ALL) ALL, !!!/bin/mount\n" =
  { "spec"
    { "user" = "%opssudoers" }
    { "host_group"
      { "host" = "ALL" }
      { "command" = "ALL"
        { "runas_user" = "ALL" } }
      { "command" = "/bin/mount"
        { "negate" } }
    }
  }
