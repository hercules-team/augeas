module Test_Cron_User =

let s = "MAILTO=cron@example.com
31 * * * * ${HOME}/bin/stuff
54 16 * * * /usr/sbin/tmpwatch -umc 30d ${HOME}/tmp\n"

let lns = Cron_User.lns

test lns get s =
  { "MAILTO" = "cron@example.com" }
  { "entry" = "${HOME}/bin/stuff"
    { "time"
      { "minute" = "31" }
      { "hour" = "*" }
      { "dayofmonth" = "*" }
      { "month" = "*" }
      { "dayofweek" = "*" }
    }
  }
  { "entry" = "/usr/sbin/tmpwatch -umc 30d ${HOME}/tmp"
    { "time"
      { "minute" = "54" }
      { "hour" = "16" }
      { "dayofmonth" = "*" }
      { "month" = "*" }
      { "dayofweek" = "*" }
    }
  }

test lns put s after
rm "/MAILTO";
rm "/entry[time/minute = '54']";
set "/entry[. = '${HOME}/bin/stuff']/time/minute" "24" =
  "24 * * * * ${HOME}/bin/stuff\n"
