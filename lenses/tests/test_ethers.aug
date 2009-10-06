(* Tests for the Ethers module *)

module Test_ethers =

(*
  let empty_entries = "# see man ethers for syntax\n"

  test Ethers.record get empty_entries =
    { "#comment" = "see man ethers for syntax" }
*)

  let three_entries = "54:52:00:01:00:01 192.168.1.1
# \tcomment\t
54:52:00:01:00:02 foo.example.com
00:16:3e:01:fe:03 bar
"

  test Ethers.lns get three_entries =
   { "1" { "mac" = "54:52:00:01:00:01" }
         { "ip" = "192.168.1.1" } }
   { "#comment" = "comment" }
   { "2" { "mac" = "54:52:00:01:00:02" }
         { "ip" = "foo.example.com" } }
   { "3" { "mac" = "00:16:3e:01:fe:03" }
         { "ip" = "bar" } }

  (* Deleting the 'ip' node violates the schema *)
  test Ethers.lns put three_entries after
      rm "/1/ip"
    = *

  test Ethers.lns put three_entries after
    set "/2/ip" "192.168.1.2" ;
    set "/3/ip" "baz"
  = "54:52:00:01:00:01 192.168.1.1
# \tcomment\t
54:52:00:01:00:02 192.168.1.2
00:16:3e:01:fe:03 baz
"

  test Ethers.lns put three_entries after
    rm "/3"
  = "54:52:00:01:00:01 192.168.1.1
# \tcomment\t
54:52:00:01:00:02 foo.example.com
"

  (* Make sure blank and indented lines get through *)
  test Ethers.lns get "54:52:00:01:00:01\tfoo  \n \n\n
54:52:00:01:00:02 bar\n" =
    { "1" { "mac" = "54:52:00:01:00:01" }
          { "ip" = "foo" } }
    {} {} {}
    { "2" { "mac" = "54:52:00:01:00:02" }
          { "ip" = "bar" } }

(* Local Variables: *)
(* mode: caml *)
(* End: *)


