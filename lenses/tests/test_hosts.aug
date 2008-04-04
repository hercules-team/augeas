(* Parsing /etc/hosts *)

module Test_hosts =

  let two_entries = "127.0.0.1 foo
# comment
192.168.0.1 pigiron.example.com pigiron pigiron.example
"

  (* Unit tests *)
  test Hosts.record get "127.0.0.1 foo" =
    { "0" { "ipaddr" = "127.0.0.1" } 
          { "canonical" = "foo" }
          { "aliases" } }

  test Hosts.top get two_entries =
   { "0" { "ipaddr" = "127.0.0.1" } 
          { "canonical" = "foo" }
          { "aliases" }
    }
    { }
    { "1" { "ipaddr" = "192.168.0.1" } 
          { "canonical" = "pigiron.example.com" }
          { "aliases"
             { "0" = "pigiron" }
             { "1" = "pigiron.example" }  }  }

  test Hosts.record put "127.0.0.1 foo" after
      set "0/canonical" "bar" 
  = "127.0.0.1 bar"

  test Hosts.top put two_entries after 
    set "1/aliases/10" "piggy" ;
    rm "1/aliases/1" 
  = "127.0.0.1 foo
# comment
192.168.0.1 pigiron.example.com pigiron piggy
"

(* Local Variables: *)
(* mode: caml *)
(* End: *)


