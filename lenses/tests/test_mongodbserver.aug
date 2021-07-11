(*
Module: Test_MongoDBServer
  Provides unit tests and examples for the <MongoDBServer> lens.
*)

module Test_MongoDBServer =

(* Variable: conf *)
let conf = "port = 27017
fork = true
pidfilepath = /var/run/mongodb/mongod.pid
logpath = /var/log/mongodb/mongod.log
dbpath =/var/lib/mongodb
journal = true
nohttpinterface = true
"

(* Test: MongoDBServer.lns *)
test MongoDBServer.lns get conf =
  { "port" = "27017" }
  { "fork" = "true" }
  { "pidfilepath" = "/var/run/mongodb/mongod.pid" }
  { "logpath" = "/var/log/mongodb/mongod.log" }
  { "dbpath" = "/var/lib/mongodb" }
  { "journal" = "true" }
  { "nohttpinterface" = "true" }

(* Test: MongoDBServer.lns
   Values have to be without quotes *)
test MongoDBServer.lns get "port = 27017\n" =
  { "port" = "27017" }
