module Test_Pagekite =

let conf1 = "# Use the pagekite.net service defaults.
defaults
"
test Pagekite.lns get conf1 =
  { "#comment" = "Use the pagekite.net service defaults." }
  { "defaults" }


let conf2 ="
frontends = pagekite.freedombox.me
ports=80,81
"
test Pagekite.lns get conf2 =
  { }
  { "frontends" = "pagekite.freedombox.me" }
  { "ports"
    { "1" = "80" }
    { "2" = "81" } }


let conf3 = "frontend=pagekite.freedombox.me
host=192.168.0.3
"
test Pagekite.lns get conf3 =
  { "frontend" = "pagekite.freedombox.me" }
  { "host" = "192.168.0.3" }


let conf4 = "isfrontend
ports=80,443
protos=http,https
domain=http,https:*.your.domain:MakeUpAPasswordHere
"
test Pagekite.lns get conf4 =
  { "isfrontend" }
  { "ports"
    { "1" = "80" }
    { "2" = "443" } }
  { "protos"
    { "1" = "http" }
    { "2" = "https" } }
  { "domain" = "http,https:*.your.domain:MakeUpAPasswordHere" }

let conf_account = "kitename = my.freedombox.me
kitesecret = 0420
# Delete this line!
abort_not_configured
"
test Pagekite.lns get conf_account =
  { "kitename" = "my.freedombox.me" }
  { "kitesecret" = "0420" }
  { "#comment" = "Delete this line!" }
  { "abort_not_configured" }
                                                                                                                                                                                        

let conf_service = "
service_on = raw/22:@kitename : localhost:22 : @kitesecret
service_on=http:192.168.0.1:127.0.0.1:80:
service_on=https:yourhostname,fqdn:127.0.0.1:443:
"
test Pagekite.lns get conf_service =
  {  }
  { "service_on"
    { "1"
      { "protocol" = "raw/22" }
      { "kitename" = "@kitename" }
      { "backend_host" = "localhost" }
      { "backend_port" = "22" }
      { "secret" = "@kitesecret" }
    }
  }
  { "service_on"
    { "2"
      { "protocol" = "http" }
      { "kitename" = "192.168.0.1" }
      { "backend_host" = "127.0.0.1" }
      { "backend_port" = "80" }
    }
  }
  { "service_on"
    { "3"
      { "protocol" = "https" }
      { "kitename" = "yourhostname,fqdn" }
      { "backend_host" = "127.0.0.1" }
      { "backend_port" = "443" }
    }
  }


let conf_encryption = "
frontend=frontend.your.domain:443
fe_certname=frontend.your/domain
ca_certs=/etc/pagekite.d/site-cert.pem
tls_endpoint=frontend.your.domain:/path/to/frontend.pem
"
test Pagekite.lns get conf_encryption =
  {  }
  { "frontend" = "frontend.your.domain:443" }
  { "fe_certname" = "frontend.your/domain" }
  { "ca_certs" = "/etc/pagekite.d/site-cert.pem" }
  { "tls_endpoint" = "frontend.your.domain:/path/to/frontend.pem" }


let conf_service_cfg = "insecure
service_cfg = KITENAME.pagekite.me/80 : insecure : True
"
test Pagekite.lns get conf_service_cfg =
  { "insecure" }
  { "service_cfg" = "KITENAME.pagekite.me/80 : insecure : True" }
