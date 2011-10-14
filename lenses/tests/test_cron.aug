module Test_cron =

   let conf = "# /etc/cron.d/anacron: crontab entries for the anacron package

SHELL=/bin/sh
PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
CRON_TZ=America/Los_Angeles
MAILTO=user1@tld1,user2@tld2;user3@tld3

	30 7      * * *   root	test -x /etc/init.d/anacron && /usr/sbin/invoke-rc.d anacron start >/dev/null
  00 */3    15-25/2 May 1-5   user   somecommand
  00 */3    15-25/2 May mon-tue   user   somecommand
# a comment
@yearly				foo    a command\n"

   test Cron.lns get conf =
      { "#comment" = "/etc/cron.d/anacron: crontab entries for the anacron package" }
      {}
      { "SHELL" = "/bin/sh" }
      { "PATH"  = "/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin" }
      { "CRON_TZ" = "America/Los_Angeles" }
      { "MAILTO" = "user1@tld1,user2@tld2;user3@tld3" }
      {}
      { "entry" = "test -x /etc/init.d/anacron && /usr/sbin/invoke-rc.d anacron start >/dev/null"
          { "time"
              { "minute"       = "30"   }
              { "hour"         = "7"    }
              { "dayofmonth"   = "*"    }
              { "month"        = "*"    }
              { "dayofweek"    = "*"    } }
          { "user"         = "root" } }
      { "entry" = "somecommand"
          { "time"
              { "minute"       = "00"      }
              { "hour"         = "*/3"     }
              { "dayofmonth"   = "15-25/2" }
              { "month"        = "May"     }
              { "dayofweek"    = "1-5"     } }
          { "user"         = "user"    } }
      { "entry" = "somecommand"
          { "time"
              { "minute"       = "00"      }
              { "hour"         = "*/3"     }
              { "dayofmonth"   = "15-25/2" }
              { "month"        = "May"     }
              { "dayofweek"    = "mon-tue"     } }
          { "user"         = "user"    } }
      { "#comment" = "a comment" }
      { "entry" = "a command"
          { "schedule"     = "yearly"  }
          { "user"         = "foo"     } }
