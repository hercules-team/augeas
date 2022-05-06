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

let save_entry_empty = "save \"\"\n"
test Redis.lns get save_entry_empty = { "save" = "" }

let replicaof_entry = "slaveof 192.168.0.10 6379\nreplicaof 192.168.0.11 6380\n"
test Redis.lns get replicaof_entry =
{ "slaveof"
  { "ip" = "192.168.0.10" }
  { "port" = "6379" } }
{ "replicaof"
  { "ip" = "192.168.0.11" }
  { "port" = "6380" } }

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

# Note on units: when memory size is needed, it is possible to specify
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
  { "#comment" = "Note on units: when memory size is needed, it is possible to specify" }
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
  { "bind" { "ip" = "127.0.0.1" } }
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

(* Test: Redis.lns
     Multiple bind IP addresses (GH issue #194) *)
test Redis.lns get "bind 127.0.0.1 \"::1\" 192.168.1.1\n" =
  { "bind"
      { "ip" = "127.0.0.1" }
      { "ip" = "::1" }
      { "ip" = "192.168.1.1" } }

test Redis.lns get "bind 127.0.0.1\n bind 192.168.1.1\n" =
  { "bind"
    { "ip" = "127.0.0.1" } }
  { "bind"
    { "ip" = "192.168.1.1" } }

let sentinel_conf = "sentinel myid ccae7d051dfaa62078cb3ac3dec100240e637d5a
sentinel deny-scripts-reconfig yes
sentinel monitor Master 8.8.8.8 6379 2
sentinel monitor Othercluster 1.1.1.1 6380 4
sentinel config-epoch Master 693
sentinel leader-epoch Master 691
sentinel known-replica Master 4.4.4.4 6379
sentinel known-replica Master 1.1.1.1 6379
sentinel known-sentinel Master 4.4.4.4 26379 9bbd89f3846b5366f7da4d20b516fdc3f5c3a993
sentinel known-sentinel Master 1.1.1.1 26379 f435adae0efeb9d5841712d05d7399f7584f333b
sentinel known-sentinel Othercluster 4.4.4.4 26379 9bbd89f3846b5366f7da4d20b516fdc3f5c3a993
sentinel known-sentinel Othercluster 1.1.1.1 26379 f435adae0efeb9d5841712d05d7399f7584f333b
sentinel current-epoch 693
"

test Redis.lns get sentinel_conf =
  { "sentinel" = "myid"
    { "value" = "ccae7d051dfaa62078cb3ac3dec100240e637d5a" } }
  { "sentinel" = "deny-scripts-reconfig"
     { "value" = "yes" } }
  { "sentinel" = "monitor"
      { "cluster" = "Master" }
      { "ip" = "8.8.8.8" }
      { "port" = "6379" }
      { "quorum" = "2" } }
  { "sentinel" = "monitor"
      { "cluster" = "Othercluster" }
      { "ip" = "1.1.1.1" }
      { "port" = "6380" }
      { "quorum" = "4" } }
  { "sentinel" = "config-epoch"
      { "cluster" = "Master" }
      { "epoch" = "693" } }
  { "sentinel" = "leader-epoch"
      { "cluster" = "Master" }
      { "epoch" = "691" } }
  { "sentinel" = "known-replica"
      { "cluster" = "Master" }
      { "ip" = "4.4.4.4" }
      { "port" = "6379" } }
  { "sentinel" = "known-replica"
      { "cluster" = "Master" }
      { "ip" = "1.1.1.1" }
      { "port" = "6379" } }
  { "sentinel" = "known-sentinel"
      { "cluster" = "Master" }
      { "ip" = "4.4.4.4" }
      { "port" = "26379" }
      { "id"   = "9bbd89f3846b5366f7da4d20b516fdc3f5c3a993" } }
  { "sentinel" = "known-sentinel"
      { "cluster" = "Master" }
      { "ip" = "1.1.1.1" }
      { "port" = "26379" }
      { "id"   = "f435adae0efeb9d5841712d05d7399f7584f333b" } }
  { "sentinel" = "known-sentinel"
      { "cluster" = "Othercluster" }
      { "ip" = "4.4.4.4" }
      { "port" = "26379" }
      { "id"   = "9bbd89f3846b5366f7da4d20b516fdc3f5c3a993" } }
  { "sentinel" = "known-sentinel"
      { "cluster" = "Othercluster" }
      { "ip" = "1.1.1.1" }
      { "port" = "26379" }
      { "id"   = "f435adae0efeb9d5841712d05d7399f7584f333b" } }
  { "sentinel" = "current-epoch"
      { "value" = "693" } }
