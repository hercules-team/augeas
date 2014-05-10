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
