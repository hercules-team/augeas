(*
Module: Test_Postgresql
  Provides unit tests and examples for the <Postgresql> lens.
*)

module Test_Postgresql =

(* "=" separator is optional *)
let missing_equal = "fsync on\n"
test Postgresql.lns get missing_equal =
  { "fsync" = "on" }
test Postgresql.lns put missing_equal after
  set "fsync" "off" = "fsync off\n"

(* extra whitespace is valid anywhere *)
let extra_whitespace = "   fsync  =    on   # trailing comment   \n"
test Postgresql.lns get extra_whitespace =
  { "fsync" = "on"
    { "#comment" = "trailing comment" }
  }
test Postgresql.lns put extra_whitespace after
  set "fsync" "off" = "   fsync  =    off   # trailing comment   \n"

(* no whitespace at all is also valid *)
let no_whitespace = "fsync=on\n"
test Postgresql.lns get no_whitespace =
  { "fsync" = "on" }
test Postgresql.lns put no_whitespace after
  set "fsync" "off" = "fsync=off\n"

(* Some settings specify a memory or time value. [...] Valid memory units are
   kB (kilobytes), MB (megabytes), and GB (gigabytes); valid time units are
   ms (milliseconds), s (seconds), min (minutes), h (hours), and d (days). *)
let numeric_suffix_quotes = "shared_buffers = 24MB
archive_timeout = 2min
deadlock_timeout = '1s'
"
test Postgresql.lns get numeric_suffix_quotes =
  { "shared_buffers" = "24MB" }
  { "archive_timeout" = "2min" }
  { "deadlock_timeout" = "1s" }
test Postgresql.lns put numeric_suffix_quotes after
  set "deadlock_timeout" "2s";
  set "max_stack_depth" "2MB";
  set "shared_buffers" "48MB" = "shared_buffers = 48MB
archive_timeout = 2min
deadlock_timeout = '2s'
max_stack_depth = '2MB'
"

(* floats and ints can be single quoted or not *)
let float_quotes = "seq_page_cost = 2.0
random_page_cost = '4.0'
vacuum_freeze_min_age = 50000000
vacuum_freeze_table_age = '150000000'
wal_buffers = -1
"
test Postgresql.lns get float_quotes =
  { "seq_page_cost" = "2.0" }
  { "random_page_cost" = "4.0" }
  { "vacuum_freeze_min_age" = "50000000" }
  { "vacuum_freeze_table_age" = "150000000" }
  { "wal_buffers" = "-1" }
test Postgresql.lns put float_quotes after
  set "seq_page_cost" "5.0";
  set "vacuum_cost_limit" "200";
  set "bgwriter_lru_multiplier" "2.0";
  set "log_temp_files" "-1";
  set "wal_buffers" "1" = "seq_page_cost = 5.0
random_page_cost = '4.0'
vacuum_freeze_min_age = 50000000
vacuum_freeze_table_age = '150000000'
wal_buffers = 1
vacuum_cost_limit = '200'
bgwriter_lru_multiplier = '2.0'
log_temp_files = '-1'
"

(* Boolean values can be written as on, off, true, false, yes, no, 1, 0 (all
   case-insensitive) or any unambiguous prefix of these. *)
let bool_quotes = "log_connections = yes
transform_null_equals = OFF
sql_inheritance = 'on'
synchronize_seqscans = 1
standard_conforming_strings = fal
"
test Postgresql.lns get bool_quotes =
  { "log_connections" = "yes" }
  { "transform_null_equals" = "OFF" }
  { "sql_inheritance" = "on" }
  { "synchronize_seqscans" = "1" }
  { "standard_conforming_strings" = "fal" }
test Postgresql.lns put bool_quotes after
  set "sql_inheritance" "off";
  set "log_lock_waits" "off" = "log_connections = yes
transform_null_equals = OFF
sql_inheritance = 'off'
synchronize_seqscans = 1
standard_conforming_strings = fal
log_lock_waits = 'off'
"

(* Strings must be single-quoted, except if they have no special character *)
let string_quotes = "listen_addresses = 'localhost'
stats_temp_directory = pg_stat_tmp
lc_messages = 'en_US.UTF-8'
log_filename = log
archive_command = 'tar \'quoted option\''
search_path = '\"$user\",public'
"
test Postgresql.lns get string_quotes =
  { "listen_addresses" = "localhost" }
  { "stats_temp_directory" = "pg_stat_tmp" }
  { "lc_messages" = "en_US.UTF-8" }
  { "log_filename" = "log" }
  { "archive_command" = "tar \'quoted option\'" }
  { "search_path" = "\"$user\",public" }
test Postgresql.lns put string_quotes after
  set "stats_temp_directory" "foo_bar";
  set "log_filename" "postgresql-%Y-%m-%d_%H%M%S.log";
  set "log_statement" "none" = "listen_addresses = 'localhost'
stats_temp_directory = foo_bar
lc_messages = 'en_US.UTF-8'
log_filename = 'postgresql-%Y-%m-%d_%H%M%S.log'
archive_command = 'tar \'quoted option\''
search_path = '\"$user\",public'
log_statement = 'none'
"

(* external files can be included more than once *)
let include_keyword = "Include 'foo.conf'
# can appear several times
Include 'bar.conf'
"
test Postgresql.lns get include_keyword =
  { "Include" = "foo.conf" }
  { "#comment" = "can appear several times" }
  { "Include" = "bar.conf" }

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
  { "data_directory" = "/var/lib/postgresql/8.4/main"
    { "#comment" = "use data in another directory" }
  }
  { "hba_file" = "/etc/postgresql/8.4/main/pg_hba.conf"
    { "#comment" = "host-based authentication file" }
  }
  { "ident_file" = "/etc/postgresql/8.4/main/pg_ident.conf"
    { "#comment" = "ident configuration file" }
  }
  {  }
  { "#comment" = "If external_pid_file is not explicitly set, no extra PID file is written." }
  { "external_pid_file" = "/var/run/postgresql/8.4-main.pid"
    { "#comment" = "write an extra PID file" }
  }
  { "listen_addresses" = "localhost"
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
  { "unix_socket_directory" = "/var/run/postgresql"
    { "#comment" = "(change requires restart)" }
  }
  { "unix_socket_group" = ""
    { "#comment" = "(change requires restart)" }
  }
  { "unix_socket_permissions" = "0777"
    { "#comment" = "begin with 0 to use octal notation" }
  }
  { "#comment" = "(change requires restart)" }
  { "bonjour_name" = ""
    { "#comment" = "defaults to the computer name" }
  }
  {  }
  { "authentication_timeout" = "1min"
    { "#comment" = "1s-600s" }
  }
  { "ssl" = "true"
    { "#comment" = "(change requires restart)" }
  }
  { "ssl_ciphers" = "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"
    { "#comment" = "allowed SSL ciphers" }
  }
  { "ssl_renegotiation_limit" = "512MB"
    { "#comment" = "amount of data between renegotiations" }
  }
  { "password_encryption" = "on" }
  { "db_user_namespace" = "off" }
  {  }
  { "search_path" = "\"$user\",public"
    { "#comment" = "schema names" }
  }
  { "default_tablespace" = ""
    { "#comment" = "a tablespace name, '' uses the default" }
  }
  { "temp_tablespaces" = ""
    { "#comment" = "a list of tablespace names, '' uses" }
  }
  {  }
  { "datestyle" = "iso, mdy" }
  { "intervalstyle" = "postgres" }
  { "timezone" = "unknown"
    { "#comment" = "actually, defaults to TZ environment" }
  }
