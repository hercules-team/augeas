(*
Module: Test_Nginx
  Provides unit tests and examples for the <Nginx> lens.
*)
module Test_nginx = 

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

    test Nginx.lns get conf =
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
      
