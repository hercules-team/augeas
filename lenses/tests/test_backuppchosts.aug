module Test_BackupPCHosts =

let conf = "host        dhcp    user      moreUsers
hostname1     0     user1     anotheruser,athirduser
hostname2     1     user2     stillanotheruser\n"

test BackupPCHosts.lns get conf =
  { "1"
    { "host" = "host" }
    { "dhcp" = "dhcp" }
    { "user" = "user" }
    { "moreusers" = "moreUsers" }
  }
  { "2"
    { "host" = "hostname1" }
    { "dhcp" = "0" }
    { "user" = "user1" }
    { "moreusers" = "anotheruser" }
    { "moreusers" = "athirduser" }
  }
  { "3"
    { "host" = "hostname2" }
    { "dhcp" = "1" }
    { "user" = "user2" }
    { "moreusers" = "stillanotheruser" }
  }
