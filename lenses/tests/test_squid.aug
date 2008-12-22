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
