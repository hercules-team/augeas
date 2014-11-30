module Test_Pgbouncer =
    let pgconfig =";; database name = connect string
;;
;; connect string params:
;;   dbname= host= port= user= password=
[databases]
; foodb over unix socket
foodb =

; redirect bardb to bazdb on localhost
bardb = host=localhost dbname=bazdb

; acceess to dest database will go with single user
forcedb = host=127.0.0.1 port=300 user=baz password=foo client_encoding=UNICODE datestyle=ISO connect_query='SELECT 1'
[pgbouncer]
;;; Administrative settings
logfile = /var/log/pgbouncer/pgbouncer.log
pidfile = /var/run/pgbouncer/pgbouncer.pid
; ip address or * which means all ip-s
listen_addr = 127.0.0.1
listen_port = 6432
;auth_file = /8.0/main/global/pg_auth
auth_file = /var/lib/pgsql/data/global/pg_auth
admin_users = postgres
server_reset_query = DISCARD ALL
"

test Pgbouncer.lns get pgconfig =
    { "#comment" = "; database name = connect string" }
    { "#comment" = ";" }
    { "#comment" = "; connect string params:" }
    { "#comment" = ";   dbname= host= port= user= password=" }
    { "databases"
        { "#comment" = "foodb over unix socket" }
        { "foodb" }
        { }
        { "#comment" = "redirect bardb to bazdb on localhost" }
        { "bardb" = "host=localhost dbname=bazdb" }
        { }
        { "#comment" = "acceess to dest database will go with single user" }
        { "forcedb" = "host=127.0.0.1 port=300 user=baz password=foo client_encoding=UNICODE datestyle=ISO connect_query='SELECT 1'" }
    }
    { "pgbouncer"
        { "#comment" = ";; Administrative settings" }
        { "logfile" = "/var/log/pgbouncer/pgbouncer.log" }
        { "pidfile" = "/var/run/pgbouncer/pgbouncer.pid" }
        { "#comment" = "ip address or * which means all ip-s" }
        { "listen_addr" = "127.0.0.1" }
        { "listen_port" = "6432" }
        { "#comment" = "auth_file = /8.0/main/global/pg_auth" }
        { "auth_file" = "/var/lib/pgsql/data/global/pg_auth" }
        { "admin_users" = "postgres" }
        { "server_reset_query" = "DISCARD ALL" }
    }
    