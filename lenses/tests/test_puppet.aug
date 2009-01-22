module Test_puppet =

   let conf = "
[main]
logdir=/var/log/puppet

  [puppetd]
  server=misspiggy.network.com
"

   test Puppet.lns get conf =
      {}
      { "main"
         { "logdir" = "/var/log/puppet" }
         {} }
      { "puppetd"
         { "server" = "misspiggy.network.com" } }

    test Puppet.lns put conf after
       set "main/vardir" "/var/lib/puppet";
       set "main/rundir" "/var/run/puppet"
    = "
[main]
logdir=/var/log/puppet

vardir=/var/lib/puppet
rundir=/var/run/puppet
  [puppetd]
  server=misspiggy.network.com
"

