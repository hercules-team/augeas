module Test_cron =

   let conf = "# /etc/cron.d/anacron: crontab entries for the anacron package

SHELL=/bin/sh
PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin

	30 7      * * *   root	test -x /etc/init.d/anacron && /usr/sbin/invoke-rc.d anacron start >/dev/null
  00 */3    15-25/2 May 1-5   user   somecommand
# a comment
@yearly				foo    a command\n"

   test Cron.lns get conf =
      { "#comment" = "/etc/cron.d/anacron: crontab entries for the anacron package" }
      {}
      { "SHELL" = "/bin/sh" }
      { "PATH"  = "/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin" }
      {}
      { "entry"
          { "minute"       = "30"   }
          { "hour"         = "7"    }
          { "dayofmonth"   = "*"    }
          { "month"        = "*"    }
          { "dayofweek"    = "*"    }
          { "user"         = "root" }
          { "command"      = "test -x /etc/init.d/anacron && /usr/sbin/invoke-rc.d anacron start >/dev/null" } }
      { "entry"
          { "minute"       = "00"      }
          { "hour"         = "*/3"     }
          { "dayofmonth"   = "15-25/2" }
          { "month"        = "May"     }
          { "dayofweek"    = "1-5"     }
          { "user"         = "user"    }
          { "command"      = "somecommand" } }
      { "#comment" = "a comment" }
      { "entry"
          { "schedule"     = "yearly"  }
          { "user"         = "foo"     }
          { "command"      = "a command" } }
