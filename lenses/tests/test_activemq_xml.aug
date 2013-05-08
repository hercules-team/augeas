(*
Module: Test_ActiveMQ_XML
  Provides unit tests and examples for the <ActiveMQ_XML> lens.
*)

module Test_ActiveMQ_XML =

(* Variable: conf *)
let conf = "<beans>
 <broker xmlns=\"http://activemq.apache.org/schema/core\" brokerName=\"localhost\" dataDirectory=\"${activemq.data}\">
  <transportConnectors>
   <transportConnector name=\"openwire\" uri=\"tcp://0.0.0.0:61616\"/>
  </transportConnectors>
 </broker>
</beans>
"

(* Variable: new_conf *) 
let new_conf = "<beans>
 <broker xmlns=\"http://activemq.apache.org/schema/core\" brokerName=\"localhost\" dataDirectory=\"${activemq.data}\">
  <transportConnectors>
   <transportConnector name=\"openwire\" uri=\"tcp://127.0.0.1:61616\"/>
  </transportConnectors>
 </broker>
</beans>
"

let lns = ActiveMQ_XML.lns 

(* Test: ActiveMQ_XML.lns  
 * Get test against tree structure
*)
test lns get conf = 
  { "beans"
    { "#text" = "
 " }
    { "broker"
      { "#attribute"
        { "xmlns" = "http://activemq.apache.org/schema/core" }
        { "brokerName" = "localhost" }
        { "dataDirectory" = "${activemq.data}" }
      }
      { "#text" = "
  " }
      { "transportConnectors"
        { "#text" = "
   " }
        { "transportConnector" = "#empty"
          { "#attribute"
            { "name" = "openwire" }
            { "uri" = "tcp://0.0.0.0:61616" }
          }
        }
        { "#text" = "  " }
      }
      { "#text" = " " }
    }
  }

(* Test: ActiveMQ_XML.lns  
 * Put test changing transport connector to localhost
*)
test lns put conf after set "/beans/broker/transportConnectors/transportConnector/#attribute/uri" "tcp://127.0.0.1:61616" = new_conf

(* vim: set ts=4  expandtab  sw=4: *)
