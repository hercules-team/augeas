module Test_Shadow =

let conf = "root:x:0:0:999999:7:::
libuuid:*:0:0:0::::
expired:$6$INVALID:0:0:0:::100:
locked:!$6$INVALID:0:0:0::::
"

test Shadow.lns get conf =
  { "root"
    { "password" = "x" }
    { "lastchange_date" = "0" }
    { "minage_days" = "0" }
    { "maxage_days" = "999999" }
    { "warn_days" = "7" }
    { "inactive_days" = "" }
    { "expire_date" = "" }
    { "flag" = "" } }
  { "libuuid"
    { "password" = "*" }
    { "lastchange_date" = "0" }
    { "minage_days" = "0" }
    { "maxage_days" = "0" }
    { "warn_days" = "" }
    { "inactive_days" = "" }
    { "expire_date" = "" }
    { "flag" = "" } }
  { "expired"
    { "password" = "$6$INVALID" }
    { "lastchange_date" = "0" }
    { "minage_days" = "0" }
    { "maxage_days" = "0" }
    { "warn_days" = "" }
    { "inactive_days" = "" }
    { "expire_date" = "100" }
    { "flag" = "" } }
  { "locked"
    { "password" = "!$6$INVALID" }
    { "lastchange_date" = "0" }
    { "minage_days" = "0" }
    { "maxage_days" = "0" }
    { "warn_days" = "" }
    { "inactive_days" = "" }
    { "expire_date" = "" }
    { "flag" = "" } }

test Shadow.lns get "+\n" =
  { "@nisdefault" }

test Shadow.lns get "+::::::::\n" =
  { "@nisdefault"
    { "password" = "" }
    { "lastchange_date" = "" }
    { "minage_days" = "" }
    { "maxage_days" = "" }
    { "warn_days" = "" }
    { "inactive_days" = "" }
    { "expire_date" = "" }
    { "flag" = "" } }

test Shadow.lns put "+\n" after
  set "@nisdefault/password" "";
  set "@nisdefault/lastchange_date" "";
  set "@nisdefault/minage_days" "";
  set "@nisdefault/maxage_days" "";
  set "@nisdefault/warn_days" "";
  set "@nisdefault/inactive_days" "";
  set "@nisdefault/expire_date" "";
  set "@nisdefault/flag" ""
= "+::::::::\n"

test Shadow.lns put "+::::::::\n" after
  rm "@nisdefault/password";
  rm "@nisdefault/lastchange_date";
  rm "@nisdefault/minage_days";
  rm "@nisdefault/maxage_days";
  rm "@nisdefault/warn_days";
  rm "@nisdefault/inactive_days";
  rm "@nisdefault/expire_date";
  rm "@nisdefault/flag"
= "+\n"
