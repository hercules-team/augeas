module Test_fail2ban =

let conf = "[DEFAULT]
mta = ssmtp
bantime = 432000
destemail = fail2ban@domain.com
findtime = 3600
maxretry = 3

[sshd]
enabled = true
"


test Fail2ban.lns get conf = 
  { "DEFAULT"
     { "mta" = "ssmtp" }
     { "bantime" = "432000" }
     { "destemail" = "fail2ban@domain.com" }
     { "findtime" = "3600" }
     { "maxretry" = "3" }
     {} }
  { "sshd"
     { "enabled" = "true" } }
