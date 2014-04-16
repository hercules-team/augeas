(*
Module: Test_Redis
  Provides unit tests and examples for the <Redis> lens.
*)

module Test_Redis =

let standard_entry = "dir /var/lib/redis\n"
test Redis.lns get standard_entry = { "dir" = "/var/lib/redis" }

let double_quoted_entry = "dir \"/var/lib/redis\"\n"
test Redis.lns get double_quoted_entry = { "dir" = "/var/lib/redis" }

let single_quoted_entry = "dir '/var/lib/redis'\n"
test Redis.lns get single_quoted_entry = { "dir" = "/var/lib/redis" }

let extra_whitespace_entry = "   dir    /var/lib/redis     \n"
test Redis.lns get extra_whitespace_entry = { "dir" = "/var/lib/redis" }

let save_entry = "save 60 10000\n"
test Redis.lns get save_entry =
{ "save"
  { "seconds" = "60" }
  { "keys" = "10000" }
}

let save_entry_quotes = "save '60' \"10000\"\n"
test Redis.lns get save_entry_quotes =
{ "save"
  { "seconds" = "60" }
  { "keys" = "10000" }
}

let slaveof_entry = "slaveof 192.168.0.10 6379\n"
test Redis.lns get slaveof_entry =
{ "slaveof"
  { "ip" = "192.168.0.10" }
  { "port" = "6379" }
}

let rename_command_entry = "rename-command CONFIG CONFIG2\n"
test Redis.lns get rename_command_entry =
{ "rename-command"
  { "from" = "CONFIG" }
  { "to" = "CONFIG2" }
}

let client_output_buffer_limit_entry_1 = "client-output-buffer-limit normal 0 0 0\n"
test Redis.lns get client_output_buffer_limit_entry_1 =
{ "client-output-buffer-limit"
  { "class" = "normal" }
  { "hard_limit" = "0" }
  { "soft_limit" = "0" }
  { "soft_seconds" = "0" }
}

let client_output_buffer_limit_entry_2 = "client-output-buffer-limit slave 256mb 64mb 60\n"
test Redis.lns get client_output_buffer_limit_entry_2 =
{ "client-output-buffer-limit"
  { "class" = "slave" }
  { "hard_limit" = "256mb" }
  { "soft_limit" = "64mb" }
  { "soft_seconds" = "60" }
}

let include_entry = "include /foo/redis.conf\ninclude /bar/redis.conf\n"
test Redis.lns get include_entry =
{ "include" = "/foo/redis.conf" }
{ "include" = "/bar/redis.conf" }

let standard_comment = "# a comment\n"
test Redis.lns get standard_comment = { "#comment" = "a comment" }

let extra_whitespace_comment = "   #     another comment        \n"
test Redis.lns get extra_whitespace_comment = { "#comment" = "another comment" }

let redis_conf = "# Redis configuration file example

# Note on units: when memory size is needed, it is possible to specifiy
# it in the usual form of 1k 5GB 4M and so forth:
#
# 1k => 1000 bytes
# 1kb => 1024 bytes
# 1m => 1000000 bytes
# 1mb => 1024*1024 bytes
# 1g => 1000000000 bytes
# 1gb => 1024*1024*1024 bytes
#
# units are case insensitive so 1GB 1Gb 1gB are all the same.

# By default Redis does not run as a daemon. Use 'yes' if you need it.
# Note that Redis will write a pid file in /var/run/redis.pid when daemonized.
daemonize yes

# When running daemonized, Redis writes a pid file in /var/run/redis.pid by
# default. You can specify a custom pid file location here.
pidfile /var/run/redis/redis-server.pid

# Accept connections on the specified port, default is 6379.
# If port 0 is specified Redis will not listen on a TCP socket.
port 6379

# If you want you can bind a single interface, if the bind option is not
# specified all the interfaces will listen for incoming connections.
#
bind 127.0.0.1

#   Note: you can disable saving at all commenting all the \"save\" lines.

save 900 1
save 300 10
save 60 10000

# Include one or more other config files here.  This is useful if you
# have a standard template that goes to all redis server but also need
# to customize a few per-server settings.  Include files can include
# other files, so use this wisely.
#
include /path/to/local.conf
include /path/to/other.conf
"

test Redis.lns get redis_conf =
  { "#comment" = "Redis configuration file example" }
  { }
  { "#comment" = "Note on units: when memory size is needed, it is possible to specifiy" }
  { "#comment" = "it in the usual form of 1k 5GB 4M and so forth:" }
  { }
  { "#comment" = "1k => 1000 bytes" }
  { "#comment" = "1kb => 1024 bytes" }
  { "#comment" = "1m => 1000000 bytes" }
  { "#comment" = "1mb => 1024*1024 bytes" }
  { "#comment" = "1g => 1000000000 bytes" }
  { "#comment" = "1gb => 1024*1024*1024 bytes" }
  { }
  { "#comment" = "units are case insensitive so 1GB 1Gb 1gB are all the same." }
  { }
  { "#comment" = "By default Redis does not run as a daemon. Use 'yes' if you need it." }
  { "#comment" = "Note that Redis will write a pid file in /var/run/redis.pid when daemonized." }
  { "daemonize" = "yes" }
  { }
  { "#comment" = "When running daemonized, Redis writes a pid file in /var/run/redis.pid by" }
  { "#comment" = "default. You can specify a custom pid file location here." }
  { "pidfile" = "/var/run/redis/redis-server.pid" }
  { }
  { "#comment" = "Accept connections on the specified port, default is 6379." }
  { "#comment" = "If port 0 is specified Redis will not listen on a TCP socket." }
  { "port" = "6379" }
  { }
  { "#comment" = "If you want you can bind a single interface, if the bind option is not" }
  { "#comment" = "specified all the interfaces will listen for incoming connections." }
  { }
  { "bind" = "127.0.0.1" }
  { }
  { "#comment" = "Note: you can disable saving at all commenting all the \"save\" lines." }
  { }
  { "save"
    { "seconds" = "900" }
    { "keys" = "1" }
  }
  { "save"
    { "seconds" = "300" }
    { "keys" = "10" }
  }
  { "save"
    { "seconds" = "60" }
    { "keys" = "10000" }
  }
  { }
  { "#comment" = "Include one or more other config files here.  This is useful if you" }
  { "#comment" = "have a standard template that goes to all redis server but also need" }
  { "#comment" = "to customize a few per-server settings.  Include files can include" }
  { "#comment" = "other files, so use this wisely." }
  { }
  { "include" = "/path/to/local.conf" }
  { "include" = "/path/to/other.conf" }

(* Test: Redis.lns
     Empty value (GH issue #115) *)
test Redis.lns get "notify-keyspace-events \"\"\n" =
  { "notify-keyspace-events" = "" }
