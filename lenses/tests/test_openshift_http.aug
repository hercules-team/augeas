(*
Module: Test_OpenShift_Http
  Provides unit tests and examples for the <OpenShift_Http> lens.
*)

module Test_OpenShift_Http =

(* Variable: conf *)
let conf = "Listen 127.0.0.1:8080
User apache
Group apache
include /etc/httpd/conf.d/ruby193-passenger.conf
PassengerUser apache
PassengerMaxPoolSize 80
PassengerMinInstances 2
PassengerPreStart http://127.0.0.1:8080/
PassengerUseGlobalQueue off
RackBaseURI /broker
PassengerRuby /var/www/openshift/broker/script/broker_ruby
<Directory /var/www/openshift/broker/httpd/root/broker>
    Options -MultiViews
</Directory>
"

(* Variable: new_conf *) 
let new_conf = "Listen 127.0.0.1:8080
User nobody
Group apache
include /etc/httpd/conf.d/ruby193-passenger.conf
PassengerUser apache
PassengerMaxPoolSize 80
PassengerMinInstances 2
PassengerPreStart http://127.0.0.1:8080/
PassengerUseGlobalQueue off
RackBaseURI /broker
PassengerRuby /var/www/openshift/broker/script/broker_ruby
<Directory /var/www/openshift/broker/httpd/root/broker>
    Options -MultiViews
</Directory>
"

let lns = OpenShift_Http.lns 

(* Test: OpenShift_Http.lns  
 * Get test against tree structure
*)
test lns get conf = 
  { "directive" = "Listen"      {"arg" = "127.0.0.1:8080" }  }
  { "directive" = "User"        { "arg" = "apache" } }
  { "directive" = "Group"       { "arg" = "apache" } }
  { "directive" = "include"     { "arg" = "/etc/httpd/conf.d/ruby193-passenger.conf" } }
  { "directive" = "PassengerUser" { "arg" = "apache" } }
  { "directive" = "PassengerMaxPoolSize" { "arg" = "80" } }
  { "directive" = "PassengerMinInstances" { "arg" = "2" } }
  { "directive" = "PassengerPreStart" { "arg" = "http://127.0.0.1:8080/" } }
  { "directive" = "PassengerUseGlobalQueue" { "arg" = "off" } }
  { "directive" = "RackBaseURI" { "arg" = "/broker" } }
  { "directive" = "PassengerRuby" { "arg" = "/var/www/openshift/broker/script/broker_ruby" } }
  { "Directory" 
    { "arg" = "/var/www/openshift/broker/httpd/root/broker" } 
    { "directive" = "Options" 
        { "arg" = "-MultiViews" }
    }
  }

(* Test: OpenShift_Http.lns  
 * Put test changing user to nobody
*)
test lns put conf after set "/directive[2]/arg" "nobody" = new_conf

(* vim: set ts=4  expandtab  sw=4: *)
