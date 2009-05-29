(* Tests for the Hosts module *)

module Test_hosts =

  let two_entries = "127.0.0.1 foo foo.example.com
  # \tcomment\t
192.168.0.1 pigiron.example.com pigiron pigiron.example
"

  test Hosts.record get "127.0.0.1 foo\n" =
    { "1" { "ipaddr" = "127.0.0.1" }
          { "canonical" = "foo" } }

  test Hosts.lns get two_entries =
   { "1" { "ipaddr" = "127.0.0.1" }
          { "canonical" = "foo" }
          { "alias" = "foo.example.com" }
    }
    { "#comment" = "comment" }
    { "2" { "ipaddr" = "192.168.0.1" }
          { "canonical" = "pigiron.example.com" }
          { "alias" = "pigiron" }
          { "alias" = "pigiron.example" }  }

  test Hosts.record put "127.0.0.1 foo\n" after
      set "/1/canonical" "bar"
  = "127.0.0.1 bar\n"

  test Hosts.lns put two_entries after
    set "/2/alias[10]" "piggy" ;
    rm "/1/alias[1]" ;
    rm "/2/alias[2]"
  = "127.0.0.1 foo
  # \tcomment\t
192.168.0.1 pigiron.example.com pigiron piggy
"

  (* Deleting the 'canonical' node violates the schema; each host entry *)
  (* must have one                                                      *)
  test Hosts.lns put two_entries after
      rm "/1/canonical"
    = *

  (* Make sure blank and indented lines get through *)
  test Hosts.lns get " \t 127.0.0.1\tlocalhost  \n \n\n
127.0.1.1\tetch.example.com\tetch\n" =
    { "1" { "ipaddr" = "127.0.0.1" }
          { "canonical" = "localhost" } }
    {} {} {}
    { "2" { "ipaddr" = "127.0.1.1" }
          { "canonical" = "etch.example.com" }
          { "alias" = "etch" } }

  (* Comment at the end of a line *)
  test Hosts.lns get "127.0.0.1 localhost # must always be there \n" =
    { "1" { "ipaddr" = "127.0.0.1" }
          { "canonical" = "localhost" }
          { "#comment" = "must always be there" } }

(* Local Variables: *)
(* mode: caml *)
(* End: *)


