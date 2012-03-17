(*
Module: Test_Qpid
  Provides unit tests and examples for the <Qpid> lens.
*)

module Test_Qpid =

(* Variable: qpidd *)
let qpidd = "# Configuration file for qpidd. Entries are of the form: 
#   name=value 

# (Note: no spaces on either side of '='). Using default settings:
# \"qpidd --help\" or \"man qpidd\" for more details.
cluster-mechanism=ANONYMOUS
auth=no
max-connections=22000
syslog-name=qpidd1
"

(* Test: Qpid.lns *)
test Qpid.lns get qpidd =
   { "#comment" = "Configuration file for qpidd. Entries are of the form:" }
   { "#comment" = "name=value" }
   { }
   { "#comment" = "(Note: no spaces on either side of '='). Using default settings:" }
   { "#comment" = "\"qpidd --help\" or \"man qpidd\" for more details." }
   { "cluster-mechanism" = "ANONYMOUS" }
   { "auth" = "no" }
   { "max-connections" = "22000" }
   { "syslog-name" = "qpidd1" }

(* Variable: qpidc *)
let qpidc = "# Configuration file for the qpid c++ client library. Entries are of
# the form:
#   name=value

ssl-cert-db=/root/certs/server_db
ssl-port=5674 
"

(* Test: Qpid.lns *)
test Qpid.lns get qpidc =
   { "#comment" = "Configuration file for the qpid c++ client library. Entries are of" }
   { "#comment" = "the form:" }
   { "#comment" = "name=value" }
   { }
   { "ssl-cert-db" = "/root/certs/server_db" }
   { "ssl-port" = "5674" }
