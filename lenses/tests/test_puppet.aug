module Test_puppet =

   let conf = "
[main]
logdir=/var/log/puppet

[puppetd]
server=misspiggy.network.com
"

   test Puppet.lns get conf =
      {}
      { "section" = "main"
         { "logdir" = "/var/log/puppet" }
         {} }
      { "section" = "puppetd"
         { "server" = "misspiggy.network.com" } }

    test Puppet.lns put conf after
       set "section[1]/vardir" "/var/lib/puppet";
       set "section[1]/rundir" "/var/run/puppet"
    = "
[main]
logdir=/var/log/puppet

vardir=/var/lib/puppet
rundir=/var/run/puppet
[puppetd]
server=misspiggy.network.com
"

