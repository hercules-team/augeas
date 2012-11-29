(*
Module: Test_Rabbitmq
  Provides unit tests and examples for the <Rabbitmq> lens.
*)
module Test_Rabbitmq =

(* Test: Rabbitmq.listeners *)
test Rabbitmq.listeners get "{ssl_listeners, [5671, {\"127.0.0.1\", 5672}]}" =
  { "ssl_listeners"
    { "value" = "5671" }
    { "tuple"
      { "value" = "127.0.0.1" }
      { "value" = "5672" } } }

(* Test: Rabbitmq.ssl_options *)
test Rabbitmq.ssl_options get "{ssl_options, [
  {cacertfile,\"/path/to/testca/cacert.pem\"},
  {certfile,\"/path/to/server/cert.pem\"},
  {keyfile,\"/path/to/server/key.pem\"},
  {verify,verify_peer},
  {fail_if_no_peer_cert,false}]}" =
  { "ssl_options"
    { "cacertfile" = "/path/to/testca/cacert.pem" }
    { "certfile" = "/path/to/server/cert.pem" }
    { "keyfile" = "/path/to/server/key.pem" }
    { "verify" = "verify_peer" }
    { "fail_if_no_peer_cert" = "false" } }

(* Test: Rabbitmq.disk_free_limit *)
test Rabbitmq.disk_free_limit get "{disk_free_limit, 1000000000}" =
  { "disk_free_limit" = "1000000000" }

(* Test: Rabbitmq.disk_free_limit *)
test Rabbitmq.disk_free_limit get "{disk_free_limit, {mem_relative, 1.0}}" =
  { "disk_free_limit"
    { "tuple"
      { "value" = "mem_relative" }
      { "value" = "1.0" } } }

(* Test: Rabbitmq.log_levels *)
test Rabbitmq.log_levels get "{log_levels, [{connection, info}]}" =
  { "log_levels"
    { "tuple"
      { "value" = "connection" }
      { "value" = "info" } } }

(* Test: Rabbitmq.cluster_nodes *)
test Rabbitmq.cluster_nodes get "{cluster_nodes, {['rabbit@rabbit1', 'rabbit@rabbit2', 'rabbit@rabbit3'], disc}}" =
  { "cluster_nodes"
    { "tuple"
      { "value"
        { "value" = "rabbit@rabbit1" }
        { "value" = "rabbit@rabbit2" }
        { "value" = "rabbit@rabbit3" } }
      { "value" = "disc" } } }

(* Test: Rabbitmq.cluster_nodes
     Apparently, tuples are not mandatory *)
test Rabbitmq.cluster_nodes get "{cluster_nodes, ['rabbit@rabbit1', 'rabbit@rabbit2', 'rabbit@rabbit3']}" =
  { "cluster_nodes"
     { "value" = "rabbit@rabbit1" }
     { "value" = "rabbit@rabbit2" }
     { "value" = "rabbit@rabbit3" } }

(* Test: Rabbitmq.lns
     Top-level test *)
test Rabbitmq.lns get "
% A standard configuration
[
  {rabbit, [
     {ssl_listeners, [5671]},
     {ssl_options, [{cacertfile,\"/path/to/testca/cacert.pem\"},
                    {certfile,\"/path/to/server/cert.pem\"},
                    {keyfile,\"/path/to/server/key.pem\"},
                    {verify,verify_peer},
                    {fail_if_no_peer_cert,false}]}
   ]}
].
% EOF\n" =
  { }
  { "#comment" = "A standard configuration" }
  { "rabbit"
    { "ssl_listeners"
      { "value" = "5671" } }
    { "ssl_options"
      { "cacertfile" = "/path/to/testca/cacert.pem" }
      { "certfile" = "/path/to/server/cert.pem" }
      { "keyfile" = "/path/to/server/key.pem" }
      { "verify" = "verify_peer" }
      { "fail_if_no_peer_cert" = "false" } } }
  { "#comment" = "EOF" }

