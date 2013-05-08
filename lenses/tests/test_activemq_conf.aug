(*
Module: Test_ActiveMQ_Conf
  Provides unit tests and examples for the <ActiveMQ_Conf> lens.
*)

module Test_ActiveMQ_Conf =

(* Variable: conf *)
let conf = "
ACTIVEMQ_HOME=/usr/share/activemq
ACTIVEMQ_BASE=${ACTIVEMQ_HOME}
"

(* Variable: new_conf *) 
let new_conf = "
ACTIVEMQ_HOME=/usr/local/share/activemq
ACTIVEMQ_BASE=${ACTIVEMQ_HOME}
"

let lns = ActiveMQ_Conf.lns 

(* Test: ActiveMQ_Conf.lns  
 * Get test against tree structure
*)
test lns get conf = 
    { }
    { "ACTIVEMQ_HOME" = "/usr/share/activemq" }
    { "ACTIVEMQ_BASE" = "${ACTIVEMQ_HOME}" }

(* Test: ActiveMQ_Conf.lns  
 * Put test changing user to nobody
*)
test lns put conf after set "/ACTIVEMQ_HOME" "/usr/local/share/activemq" = new_conf

(* vim: set ts=4  expandtab  sw=4: *)
