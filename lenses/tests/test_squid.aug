module Test_squid =

let conf = "# comment at the beginning of the file

auth_param negotiate children 5
acl many_spaces rep_header Content-Disposition -i [[:space:]]{3,}
acl CONNECT method CONNECT
# comment in the middle
  acl local_network src 192.168.1.0/24

http_access allow manager localhost
http_access allow local_network
"

test Squid.lns get conf =
  { "#comment" = "comment at the beginning of the file" }
  {}
  { "auth_param"
     { "scheme" = "negotiate" }
     { "parameter" = "children" }
     { "setting" = "5" } }
  { "acl"
     { "many_spaces"
        { "type" = "rep_header" }
        { "setting" = "Content-Disposition" }
        { "parameters"
           { "1" = "-i"  }
	   { "2" = "[[:space:]]{3,}" } } } }
  { "acl"
     { "CONNECT"
        { "type" = "method" }
        { "setting" = "CONNECT" } } }
  { "#comment" = "comment in the middle" }
  { "acl"
     { "local_network"
        { "type" = "src" }
        { "setting" = "192.168.1.0/24" } } }
  {}
  { "http_access"
     { "allow" = "manager"
           { "parameters"
   	        { "1" = "localhost" } } } }
  { "http_access"
     { "allow" = "local_network" } }

(*
  This tests the Debian lenny default squid.conf
  Comments were stripped out
*)

let debian_lenny_default = "acl all src all
acl manager proto cache_object
acl localhost src 127.0.0.1/32
acl to_localhost dst 127.0.0.0/8
acl purge method PURGE
acl CONNECT method CONNECT
http_access allow manager localhost
http_access deny manager
http_access allow purge localhost
http_access deny purge
http_access deny !Safe_ports
http_access deny CONNECT !SSL_ports
http_access allow localhost
http_access deny all
no_cache deny query_no_cache
icp_access allow localnet
icp_access deny all
http_port 3128
hierarchy_stoplist cgi-bin ?
access_log /var/log/squid/access.log squid
refresh_pattern ^ftp:           1440    20%     10080
refresh_pattern ^gopher:        1440    0%      1440
refresh_pattern -i (/cgi-bin/|\?) 0     0%      0
refresh_pattern (Release|Package(.gz)*)$        0       20%     2880
refresh_pattern .               0       20%     4320	ignore-reload ignore-auth # testing options
acl shoutcast rep_header X-HTTP09-First-Line ^ICY\s[0-9]
upgrade_http0.9 deny shoutcast
acl apache rep_header Server ^Apache
broken_vary_encoding allow apache
extension_methods REPORT MERGE MKACTIVITY CHECKOUT
hosts_file /etc/hosts
coredump_dir /var/spool/squid
"

test Squid.lns get debian_lenny_default =
  { "acl"
    { "all"
      { "type" = "src" }
      { "setting" = "all" }
    }
  }
  { "acl"
    { "manager"
      { "type" = "proto" }
      { "setting" = "cache_object" }
    }
  }
  { "acl"
    { "localhost"
      { "type" = "src" }
      { "setting" = "127.0.0.1/32" }
    }
  }
  { "acl"
    { "to_localhost"
      { "type" = "dst" }
      { "setting" = "127.0.0.0/8" }
    }
  }
  { "acl"
    { "purge"
      { "type" = "method" }
      { "setting" = "PURGE" }
    }
  }
  { "acl"
    { "CONNECT"
      { "type" = "method" }
      { "setting" = "CONNECT" }
    }
  }
  { "http_access"
    { "allow" = "manager"
      { "parameters"
        { "1" = "localhost" }
      }
    }
  }
  { "http_access"
    { "deny" = "manager" }
  }
  { "http_access"
    { "allow" = "purge"
      { "parameters"
        { "1" = "localhost" }
      }
    }
  }
  { "http_access"
    { "deny" = "purge" }
  }
  { "http_access"
    { "deny" = "!Safe_ports" }
  }
  { "http_access"
    { "deny" = "CONNECT"
      { "parameters"
        { "1" = "!SSL_ports" }
      }
    }
  }
  { "http_access"
    { "allow" = "localhost" }
  }
  { "http_access"
    { "deny" = "all" }
  }
  { "no_cache" =  "deny query_no_cache" }
  { "icp_access" = "allow localnet" }
  { "icp_access" = "deny all" }
  { "http_port" = "3128" }
  { "hierarchy_stoplist" = "cgi-bin ?" }
  { "access_log" = "/var/log/squid/access.log squid" }
  { "refresh_pattern" = "^ftp:"
       { "min" = "1440" }
       { "percent" = "20" }
       { "max" = "10080" } }
  { "refresh_pattern" = "^gopher:"
       { "min" = "1440" }
       { "percent" = "0" }
       { "max" = "1440" } }
  { "refresh_pattern" = "(/cgi-bin/|\?)"
       { "case_insensitive" }
       { "min" = "0" }
       { "percent" = "0" }
       { "max" = "0" } }
  { "refresh_pattern" = "(Release|Package(.gz)*)$"
       { "min" = "0" }
       { "percent" = "20" }
       { "max" = "2880" } }
  { "refresh_pattern" = "."
       { "min" = "0" }
       { "percent" = "20" }
       { "max" = "4320" }
       { "option" = "ignore-reload" }
       { "option" = "ignore-auth" }
       { "#comment" = "testing options" } }
  { "acl"
    { "shoutcast"
      { "type" = "rep_header" }
      { "setting" = "X-HTTP09-First-Line" }
      { "parameters"
        { "1" = "^ICY\s[0-9]" }
      }
    }
  }
  { "upgrade_http0.9"
      { "deny" = "shoutcast" } }
  { "acl"
    { "apache"
      { "type" = "rep_header" }
      { "setting" = "Server" }
      { "parameters"
        { "1" = "^Apache" }
      }
    }
  }
  { "broken_vary_encoding"
      { "allow" = "apache" } }
  { "extension_methods"
      { "1" = "REPORT" }
      { "2" = "MERGE" }
      { "3" = "MKACTIVITY" }
      { "4" = "CHECKOUT" } }
  { "hosts_file" = "/etc/hosts" }
  { "coredump_dir" = "/var/spool/squid" }
