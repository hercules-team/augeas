(*
Module: Test_Postgresql
  Provides unit tests and examples for the <Postgresql> lens.
*)

module Test_Postgresql =

(* Variable: conf
   A full configuration file *)
   let conf = "data_directory = '/var/lib/postgresql/8.4/main'		# use data in another directory
hba_file = '/etc/postgresql/8.4/main/pg_hba.conf'	# host-based authentication file
ident_file = '/etc/postgresql/8.4/main/pg_ident.conf'	# ident configuration file

# If external_pid_file is not explicitly set, no extra PID file is written.
external_pid_file = '/var/run/postgresql/8.4-main.pid'		# write an extra PID file
listen_addresses = 'localhost'		# what IP address(es) to listen on;
port = 5432				# (change requires restart)
max_connections = 100			# (change requires restart)
superuser_reserved_connections = 3	# (change requires restart)
unix_socket_directory = '/var/run/postgresql'		# (change requires restart)
unix_socket_group = ''			# (change requires restart)
unix_socket_permissions = 0777		# begin with 0 to use octal notation
					# (change requires restart)
bonjour_name = ''			# defaults to the computer name

authentication_timeout = 1min		# 1s-600s
ssl = true				# (change requires restart)
ssl_ciphers = 'ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH'	# allowed SSL ciphers
ssl_renegotiation_limit = 512MB	# amount of data between renegotiations
password_encryption = on
db_user_namespace = off

search_path = '\"$user\",public'		# schema names
default_tablespace = ''		# a tablespace name, '' uses the default
temp_tablespaces = ''			# a list of tablespace names, '' uses

datestyle = 'iso, mdy'
intervalstyle = 'postgres'
timezone = unknown			# actually, defaults to TZ environment
"

(* Test: Postgresql.lns *)
test Postgresql.lns get conf =
  { "data_directory" = "'/var/lib/postgresql/8.4/main'"
    { "#comment" = "use data in another directory" }
  }
  { "hba_file" = "'/etc/postgresql/8.4/main/pg_hba.conf'"
    { "#comment" = "host-based authentication file" }
  }
  { "ident_file" = "'/etc/postgresql/8.4/main/pg_ident.conf'"
    { "#comment" = "ident configuration file" }
  }
  {  }
  { "#comment" = "If external_pid_file is not explicitly set, no extra PID file is written." }
  { "external_pid_file" = "'/var/run/postgresql/8.4-main.pid'"
    { "#comment" = "write an extra PID file" }
  }
  { "listen_addresses" = "'localhost'"
    { "#comment" = "what IP address(es) to listen on;" }
  }
  { "port" = "5432"
    { "#comment" = "(change requires restart)" }
  }
  { "max_connections" = "100"
    { "#comment" = "(change requires restart)" }
  }
  { "superuser_reserved_connections" = "3"
    { "#comment" = "(change requires restart)" }
  }
  { "unix_socket_directory" = "'/var/run/postgresql'"
    { "#comment" = "(change requires restart)" }
  }
  { "unix_socket_group" = "''"
    { "#comment" = "(change requires restart)" }
  }
  { "unix_socket_permissions" = "0777"
    { "#comment" = "begin with 0 to use octal notation" }
  }
  { "#comment" = "(change requires restart)" }
  { "bonjour_name" = "''"
    { "#comment" = "defaults to the computer name" }
  }
  {  }
  { "authentication_timeout" = "1min"
    { "#comment" = "1s-600s" }
  }
  { "ssl" = "true"
    { "#comment" = "(change requires restart)" }
  }
  { "ssl_ciphers" = "'ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH'"
    { "#comment" = "allowed SSL ciphers" }
  }
  { "ssl_renegotiation_limit" = "512MB"
    { "#comment" = "amount of data between renegotiations" }
  }
  { "password_encryption" = "on" }
  { "db_user_namespace" = "off" }
  {  }
  { "search_path" = "'\"$user\",public'"
    { "#comment" = "schema names" }
  }
  { "default_tablespace" = "''"
    { "#comment" = "a tablespace name, '' uses the default" }
  }
  { "temp_tablespaces" = "''"
    { "#comment" = "a list of tablespace names, '' uses" }
  }
  {  }
  { "datestyle" = "'iso, mdy'" }
  { "intervalstyle" = "'postgres'" }
  { "timezone" = "unknown"
    { "#comment" = "actually, defaults to TZ environment" }
  }
