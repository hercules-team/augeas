module Test_mysql =

   let conf = "# The MySQL database server configuration file.
#
# You can copy this to one of:
# 
# One can use all long options that the program supports.
# Run program with --help to get a list of available options and with
# --print-defaults to see which it would actually understand and use.
#
# For explanations see
# http://dev.mysql.com/doc/mysql/en/server-system-variables.html

# This will be passed to all mysql clients
# It has been reported that passwords should be enclosed with ticks/quotes
# Remember to edit /etc/mysql/debian.cnf when changing the socket location.
 [client]
 port		= 3306
socket		= /var/run/mysqld/mysqld.sock

# Here is entries for some specific programs
# The following values assume you have at least 32M ram

# This was formally known as [safe_mysqld]. Both versions are currently parsed.
   [mysqld_safe]
socket		= /var/run/mysqld/mysqld.sock
nice		= 0

[mysqld]
#
# * Basic Settings
#
user		= mysql
pid-file	= /var/run/mysqld/mysqld.pid
socket		= /var/run/mysqld/mysqld.sock
port		= 3306
basedir		= /usr
datadir		= /var/lib/mysql
tmpdir		= /tmp
language	= /usr/share/mysql/english
 skip-external-locking
#
# Instead of skip-networking the default is now to listen only on
# localhost which is more compatible and is not less secure.
bind-address		= 127.0.0.1
#
# * Fine Tuning
#
key_buffer		= 16M
max_allowed_packet	= 16M
thread_stack		= 128K
thread_cache_size	= 8
#max_connections        = 100
#table_cache            = 64
#thread_concurrency     = 10
#
# * Query Cache Configuration
#
query_cache_limit       = 1M
query_cache_size        = 16M
#
# * Logging and Replication
#
# Both location gets rotated by the cronjob.
# Be aware that this log type is a performance killer.
#log		= /var/log/mysql/mysql.log
#
# Error logging goes to syslog. This is a Debian improvement :)
#
# Here you can see queries with especially long duration
#log_slow_queries	= /var/log/mysql/mysql-slow.log
#long_query_time = 2
#log-queries-not-using-indexes
#
# The following can be used as easy to replay backup logs or for replication.
#server-id		= 1
#log_bin		= /var/log/mysql/mysql-bin.log
# WARNING: Using expire_logs_days without bin_log crashes the server! See README.Debian!
#expire_logs_days	= 10
#max_binlog_size	= 100M
#binlog_do_db		= include_database_name
#binlog_ignore_db	= include_database_name
#
# * BerkeleyDB
#
# Using BerkeleyDB is now discouraged as its support will cease in 5.1.12.
skip-bdb
#
# * InnoDB
#
# InnoDB is enabled by default with a 10MB datafile in /var/lib/mysql/.
# Read the manual for more InnoDB related options. There are many!
# You might want to disable InnoDB to shrink the mysqld process by circa 100MB.
#skip-innodb
#
# * Security Features
#
# Read the manual, too, if you want chroot!
# chroot = /var/lib/mysql/
#
#
# ssl-ca=/etc/mysql/cacert.pem
# ssl-cert=/etc/mysql/server-cert.pem
# ssl-key=/etc/mysql/server-key.pem



[mysqldump]
quick
quote-names
max_allowed_packet	= 16M

[mysql]
#no-auto-rehash	# faster start of mysql but no tab completition

[isamchk]
key_buffer		= 16M

#
# * NDB Cluster
#
# See /usr/share/doc/mysql-server-*/README.Debian for more information.
#
# The following configuration is read by the NDB Data Nodes (ndbd processes)
# not from the NDB Management Nodes (ndb_mgmd processes).
#
# [MYSQL_CLUSTER]
# ndb-connectstring=127.0.0.1


#
# * IMPORTANT: Additional settings that can override those from this file!
#
!includedir /etc/mysql/conf.d/
!include /etc/mysql/other_conf.d/someconf.cnf
# Another comment

"


   test MySQL.lns get conf =   { "#comment" = "The MySQL database server configuration file." }
  {}
  { "#comment" = "You can copy this to one of:" }
  {}
  { "#comment" = "One can use all long options that the program supports." }
  { "#comment" = "Run program with --help to get a list of available options and with" }
  { "#comment" = "--print-defaults to see which it would actually understand and use." }
  {}
  { "#comment" = "For explanations see" }
  { "#comment" = "http://dev.mysql.com/doc/mysql/en/server-system-variables.html" }
  {  }
  { "#comment" = "This will be passed to all mysql clients" }
  { "#comment" = "It has been reported that passwords should be enclosed with ticks/quotes" }
  { "#comment" = "Remember to edit /etc/mysql/debian.cnf when changing the socket location." }
  { "target" = "client"
    { "port" = "3306" }
    { "socket" = "/var/run/mysqld/mysqld.sock" }
    {  }
    { "#comment" = "Here is entries for some specific programs" }
    { "#comment" = "The following values assume you have at least 32M ram" }
    {  }
    { "#comment" = "This was formally known as [safe_mysqld]. Both versions are currently parsed." }
  }
  { "target" = "mysqld_safe"
    { "socket" = "/var/run/mysqld/mysqld.sock" }
    { "nice" = "0" }
    {  }
  }
  { "target" = "mysqld"
    {}
    { "#comment" = "* Basic Settings" }
    {}
    { "user" = "mysql" }
    { "pid-file" = "/var/run/mysqld/mysqld.pid" }
    { "socket" = "/var/run/mysqld/mysqld.sock" }
    { "port" = "3306" }
    { "basedir" = "/usr" }
    { "datadir" = "/var/lib/mysql" }
    { "tmpdir" = "/tmp" }
    { "language" = "/usr/share/mysql/english" }
    { "skip-external-locking" = "" }
    {}
    { "#comment" = "Instead of skip-networking the default is now to listen only on" }
    { "#comment" = "localhost which is more compatible and is not less secure." }
    { "bind-address" = "127.0.0.1" }
    {}
    { "#comment" = "* Fine Tuning" }
    {}
    { "key_buffer" = "16M" }
    { "max_allowed_packet" = "16M" }
    { "thread_stack" = "128K" }
    { "thread_cache_size" = "8" }
    { "#comment" = "max_connections        = 100" }
    { "#comment" = "table_cache            = 64" }
    { "#comment" = "thread_concurrency     = 10" }
    {}
    { "#comment" = "* Query Cache Configuration" }
    {}
    { "query_cache_limit" = "1M" }
    { "query_cache_size" = "16M" }
    {}
    { "#comment" = "* Logging and Replication" }
    {}
    { "#comment" = "Both location gets rotated by the cronjob." }
    { "#comment" = "Be aware that this log type is a performance killer." }
    { "#comment" = "log		= /var/log/mysql/mysql.log" }
    {}
    { "#comment" = "Error logging goes to syslog. This is a Debian improvement :)" }
    {}
    { "#comment" = "Here you can see queries with especially long duration" }
    { "#comment" = "log_slow_queries	= /var/log/mysql/mysql-slow.log" }
    { "#comment" = "long_query_time = 2" }
    { "#comment" = "log-queries-not-using-indexes" }
    {}
    { "#comment" = "The following can be used as easy to replay backup logs or for replication." }
    { "#comment" = "server-id		= 1" }
    { "#comment" = "log_bin		= /var/log/mysql/mysql-bin.log" }
    { "#comment" = "WARNING: Using expire_logs_days without bin_log crashes the server! See README.Debian!" }
    { "#comment" = "expire_logs_days	= 10" }
    { "#comment" = "max_binlog_size	= 100M" }
    { "#comment" = "binlog_do_db		= include_database_name" }
    { "#comment" = "binlog_ignore_db	= include_database_name" }
    {}
    { "#comment" = "* BerkeleyDB" }
    {}
    { "#comment" = "Using BerkeleyDB is now discouraged as its support will cease in 5.1.12." }
    { "skip-bdb" = "" }
    {}
    { "#comment" = "* InnoDB" }
    {}
    { "#comment" = "InnoDB is enabled by default with a 10MB datafile in /var/lib/mysql/." }
    { "#comment" = "Read the manual for more InnoDB related options. There are many!" }
    { "#comment" = "You might want to disable InnoDB to shrink the mysqld process by circa 100MB." }
    { "#comment" = "skip-innodb" }
    {}
    { "#comment" = "* Security Features" }
    {}
    { "#comment" = "Read the manual, too, if you want chroot!" }
    { "#comment" = "chroot = /var/lib/mysql/" }
    {}
    {}
    { "#comment" = "ssl-ca=/etc/mysql/cacert.pem" }
    { "#comment" = "ssl-cert=/etc/mysql/server-cert.pem" }
    { "#comment" = "ssl-key=/etc/mysql/server-key.pem" }
    {  }
    {  }
    {  }
  }
  { "target" = "mysqldump"
    { "quick" = "" }
    { "quote-names" = "" }
    { "max_allowed_packet" = "16M" }
    {  }
  }
  { "target" = "mysql"
    { "#comment" = "no-auto-rehash	# faster start of mysql but no tab completition" }
    {  }
  }
  { "target" = "isamchk"
    { "key_buffer" = "16M" }
    {  }
    {}
    { "#comment" = "* NDB Cluster" }
    {}
    { "#comment" = "See /usr/share/doc/mysql-server-*/README.Debian for more information." }
    {}
    { "#comment" = "The following configuration is read by the NDB Data Nodes (ndbd processes)" }
    { "#comment" = "not from the NDB Management Nodes (ndb_mgmd processes)." }
    {}
    { "#comment" = "[MYSQL_CLUSTER]" }
    { "#comment" = "ndb-connectstring=127.0.0.1" }
    {  }
    {  }
    {}
    { "#comment" = "* IMPORTANT: Additional settings that can override those from this file!" }
    {}
  }
  { "!includedir" = "/etc/mysql/conf.d/" }
  { "!include" = "/etc/mysql/other_conf.d/someconf.cnf" }
  { "#comment" = "Another comment" }
  { }


