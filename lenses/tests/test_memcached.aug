(*
Module: Test_Memcached
  Provides unit tests and examples for the <Memcached> lens.
*)

module Test_Memcached =

let conf = "# memcached default config file

# Run memcached as a daemon. This command is implied, and is not needed for the
# daemon to run. See the README.Debian that comes with this package for more
# information.
-d
-l 127.0.0.1

# Log memcached's output to /var/log/memcached
logfile /var/log/memcached.log

# Default connection port is 11211
-p 11211    
-m    64    #    Start with a cap of 64 megs of memory.
-M
"

test Memcached.lns get conf =
  { "#comment" = "memcached default config file" }
  { }
  { "#comment" = "Run memcached as a daemon. This command is implied, and is not needed for the" }
  { "#comment" = "daemon to run. See the README.Debian that comes with this package for more" }
  { "#comment" = "information." }
  { "d" }
  { "l" = "127.0.0.1" }
  { }
  { "#comment" = "Log memcached's output to /var/log/memcached" }
  { "logfile"  = "/var/log/memcached.log" }
  { }
  { "#comment" = "Default connection port is 11211" }
  { "p" = "11211" }
  { "m" = "64"
    { "#comment" = "Start with a cap of 64 megs of memory." }
  }
  { "M" }

