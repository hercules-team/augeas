(* Tests for the Postfix Access module *)

module Test_postfix_access =

  let three_entries = "127.0.0.1  DISCARD You totally suck
   Really
#ok no more comments
user@ REJECT
"

  test Postfix_access.record get "127.0.0.1 REJECT\n" =
    { "1" { "pattern" = "127.0.0.1" }
          { "action" = "REJECT" } }

  test Postfix_access.lns get three_entries =
    { "1" { "pattern" = "127.0.0.1" }
          { "action" = "DISCARD" }
          { "parameters" = "You totally suck\n   Really" } }
    {"#comment" = "ok no more comments" }
    { "2" { "pattern" = "user@" }
          { "action" = "REJECT" } }

  test Postfix_access.record put "127.0.0.1 OK\n" after
      set "/1/action" "REJECT"
  = "127.0.0.1 REJECT\n"

  test Postfix_access.lns put three_entries after
    set "/2/parameters" "Rejected you loser" ;
    rm "/1/parameters"
  = "127.0.0.1  DISCARD
#ok no more comments
user@ REJECT Rejected you loser
"

(* Deleting the 'action' node violates the schema; each postfix access *)
(* entry must have one                                                 *)
  test Postfix_access.lns put three_entries after
      rm "/1/action"
    = *

  (* Make sure blank lines get through *)
  test Postfix_access.lns get "127.0.0.1\tREJECT  \n \n\n
user@*\tOK\tI 'll let you in \n\tseriously\n" =
    { "1" { "pattern" = "127.0.0.1" }
          { "action" = "REJECT" } }
    {} {} {}
    { "2" { "pattern" = "user@*" }
          { "action" = "OK" }
          { "parameters" = "I 'll let you in \n\tseriously" } }

(* Local Variables: *)
(* mode: caml *)
(* End: *)
