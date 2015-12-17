(*
Module: Test_Nginx
  Provides unit tests and examples for the <Nginx> lens.
*)
module Test_nginx =

(* Check for non-recursive ambiguities *)
let directive = Nginx.simple
              | Nginx.block (
                   Nginx.simple
                 | Nginx.block Nginx.simple
              )

(* Do some limited typechecking on the recursive lens; note that
   unrolling once more leads to a typecheck error that seems to
   be spurious, though it's not clear why

   Unrolling once more amounts to adding the clause
      Nginx.block (Nginx.block Nginx.simple)
   to unrolled and results in an error
      overlapping lenses in union.get
        Example matched by both: 'upstream{}\n'
*)
let unrolled = Nginx.simple | Nginx.block Nginx.simple

let lns_unrolled = (Util.comment | Util.empty | unrolled)

(* Normal unit tests *)
let lns = Nginx.lns

let conf ="user nginx nginx;
worker_processes 1;
error_log /var/log/nginx/error_log info;

events {
        worker_connections 1024;
        use epoll;
}

# comment1
# comment2

http {
	# comment3
        include /etc/nginx/mime.types;
        default_type application/octet-stream;
        log_format main
                '$remote_addr - $remote_user [$time_local] '
                '\"$request\" $status $bytes_sent '
                '\"$http_referer\" \"$http_user_agent\" '
                '\"$gzip_ratio\"';
        client_header_timeout 10m;
        client_body_timeout 10m;
        send_timeout 10m;
        connection_pool_size 256;
        client_header_buffer_size 2k;
        large_client_header_buffers 4 8k;
        request_pool_size 4k;
        gzip on;
	gzip_min_length 1000;
        gzip_buffers 4 8k;
        gzip_types text/plain application/json;
        output_buffers 1 32k;
        postpone_output 1460;
        sendfile on;
        tcp_nopush on;
        tcp_nodelay on;
        keepalive_timeout 75 20;
        ignore_invalid_headers on;
        index index.html index.php;
        include vhosts/*.conf;
}
"

test lns get conf =
   { "user"  = "nginx nginx" }
   { "worker_processes"      = "1" }
   { "error_log" = "/var/log/nginx/error_log info" }
   {}
   { "events"
      { "worker_connections"  = "1024" }
      { "use"      = "epoll" } }
   {}
   { "#comment" = "comment1" }
   { "#comment" = "comment2" }
   {}
   { "http"
   	  { "#comment" = "comment3" }
      { "include"  = "/etc/nginx/mime.types" }
      { "default_type"  = "application/octet-stream" }
      { "log_format"      = "main
                '$remote_addr - $remote_user [$time_local] '
                '\"$request\" $status $bytes_sent '
                '\"$http_referer\" \"$http_user_agent\" '
                '\"$gzip_ratio\"'" }
      { "client_header_timeout"  = "10m" }
      { "client_body_timeout"  = "10m" }
      { "send_timeout"      = "10m" }
      { "connection_pool_size"      = "256" }
      { "client_header_buffer_size"      = "2k" }
      { "large_client_header_buffers"      = "4 8k" }
      { "request_pool_size"      = "4k" }
      { "gzip"      = "on" }
      { "gzip_min_length"      = "1000" }
      { "gzip_buffers"      = "4 8k" }
      { "gzip_types"      = "text/plain application/json" }
      { "output_buffers"      = "1 32k" }
      { "postpone_output"      = "1460" }
      { "sendfile"      = "on" }
      { "tcp_nopush"      = "on" }
      { "tcp_nodelay"      = "on" }
      { "keepalive_timeout"      = "75 20" }
      { "ignore_invalid_headers"      = "on" }
      { "index"      = "index.html index.php" }
      { "include"      = "vhosts/*.conf" } }

(* location blocks *)
test lns get "location / { }\n" =
  { "location"
    { "#uri" = "/" } }

test lns get "location = / { }\n" =
  { "location"
      { "#comp" = "=" }
      { "#uri" = "/" } }

test lns get "location /documents/ { }\n" =
  { "location"
    { "#uri" = "/documents/" } }

test lns get "location ^~ /images/ { }\n" =
  { "location"
    { "#comp" = "^~" }
    { "#uri" = "/images/" } }

test lns get "location ~* \.(gif|jpg|jpeg)$ { }\n" =
  { "location"
    { "#comp" = "~*" }
    { "#uri" = "\.(gif|jpg|jpeg)$" } }

test lns get "location @fallback { }\n" =
 { "location"
    { "#uri" = "@fallback" } }

(* if blocks *)
test lns get "if ($slow) {
  tcp_nodelay on;
}\n" =
  { "if"
    { "#cond" = "($slow)" }
    { "tcp_nodelay" = "on" } }

test lns get "if ($request_method = POST)   { }\n" =
 { "if"
    { "#cond" = "($request_method = POST)" } }


test lns get "if ($http_cookie ~* \"id=([^;]+)(?:;|$)\") { }\n" =
 { "if"
    { "#cond" = "($http_cookie ~* \"id=([^;]+)(?:;|$)\")" } }

(* geo blocks *)
test lns get "geo $geo { }\n" =
  { "geo"
    { "#geo" = "$geo" } }

test lns get "geo $address $geo { }\n" =
  { "geo"
    { "#address" = "$address" }
    { "#geo" = "$geo" } }

(* map blocks *)
test lns get "map $http_host $name { }\n" =
  { "map"
    { "#source" = "$http_host" }
    { "#variable" = "$name" } }

(* split_clients block *)
test lns get "split_clients \"${remote_addr}AAA\" $variable { }\n" =
  { "split_clients"
    { "#string" = "\"${remote_addr}AAA\"" }
    { "#variable" = "$variable" } }

(* upstream block *)
test lns get "upstream backend { }\n" =
 { "upstream"
    { "#name" = "backend" } }

(* GH #179 - recursive blocks *)
let http = "http {
  server {
    listen 80;
    location / {
      root\thtml;
    }
  }
  gzip on;
}\n"

test lns get http =
  { "http"
    { "server"
       { "listen" = "80" }
       { "location"
         { "#uri" = "/" }
         { "root" = "html" } } }
    { "gzip" = "on" } }


(* GH #335 - server single line entries *)
let http_server_single_line_entries = "http {
  upstream big_server_com {
    server 127.0.0.3:8000 weight=5;
    server 127.0.0.3:8001 weight=5;
    server 192.168.0.1:8000;
    server 192.168.0.1:8001;
    server backend2.example.com:8080 fail_timeout=5s slow_start=30s;
    server backend3.example.com      resolve;
  }
}\n"

test lns get http_server_single_line_entries =
  { "http"
    { "upstream"
      { "#name" = "big_server_com" }
      { "@server" { "@address" = "127.0.0.3:8000" } { "weight" = "5" } }
      { "@server" { "@address" = "127.0.0.3:8001" } { "weight" = "5" } }
      { "@server" { "@address" = "192.168.0.1:8000" } }
      { "@server" { "@address" = "192.168.0.1:8001" } }
      { "@server"
        { "@address" = "backend2.example.com:8080" }
        { "fail_timeout" = "5s" }
        { "slow_start" = "30s" } }
      { "@server"
        { "@address" = "backend3.example.com" }
        { "resolve" } } } }

(* Make sure we do not screw up the indentation of the file *)
test lns put http after set "/http/gzip" "off" =
"http {
  server {
    listen 80;
    location / {
      root\thtml;
    }
  }
  gzip off;
}\n"

(* Test lns
     GH #260 *)
test lns get "http {
  geo $geo {
    default        0;

    127.0.0.1      2;
    192.168.1.0/24 1;
    10.1.0.0/16    1;

    ::1            2;
    2001:0db8::/32 1;
  }
}\n" =
  { "http"
    { "geo"
      { "#geo" = "$geo" }
      { "default" = "0" }
      {  }
      { "127.0.0.1" = "2" }
      { "192.168.1.0" = "1"
        { "mask" = "24" } }
      { "10.1.0.0" = "1"
        { "mask" = "16" } }
      {  }
      { "::1" = "2" }
      { "2001:0db8::" = "1"
        { "mask" = "32" } } } }
