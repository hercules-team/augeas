module Test_rsyncd =

  let conf = "
# A more sophisticated example would be:

uid = nobody
	gid = nobody
use chroot = yes
max connections = 4
syslog facility = local5
pid file = /var/run/rsyncd.pid

[ftp]
  # this is a comment
  path = /var/ftp/./pub
  comment = whole ftp area (approx 6.1 GB)

   [cvs]
  ; comment with semicolon
  path = /data/cvs
  comment = CVS repository (requires authentication)
  auth users = tridge, susan # comment at EOL
  secrets file = /etc/rsyncd.secrets

"

  test Rsyncd.lns get conf =
    { ".anon"
      {}
      { "#comment" = "A more sophisticated example would be:" }
      {}
      { "uid" = "nobody" }
      { "gid" = "nobody" }
      { "use chroot" = "yes" }
      { "max connections" = "4" }
      { "syslog facility" = "local5" }
      { "pid file" = "/var/run/rsyncd.pid" }
      {}
    }
    { "ftp"
      { "#comment" = "this is a comment" }
      { "path" = "/var/ftp/./pub" }
      { "comment" = "whole ftp area (approx 6.1 GB)" }
      {}
    }
    { "cvs"
      { "#comment" = "comment with semicolon" }
      { "path" = "/data/cvs" }
      { "comment" = "CVS repository (requires authentication)" }
      { "auth users" = "tridge, susan"
        { "#comment" = "comment at EOL" }
      }
      { "secrets file" = "/etc/rsyncd.secrets" }
      {}
    }

