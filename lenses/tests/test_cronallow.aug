module Test_cronallow =

  let example = "# Test comment
#
user1
another

user2
"

  test CronAllow.lns get example =
    { "#comment" = "Test comment" }
    { }
    { "1" = "user1" }
    { "2" = "another" }
    { }
    { "3" = "user2" }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
