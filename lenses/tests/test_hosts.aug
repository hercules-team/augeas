(* Tests for the Hosts module *)

module Test_hosts =

  let two_entries = "127.0.0.1 foo foo.example.com
# comment
192.168.0.1 pigiron.example.com pigiron pigiron.example
"

  test Hosts.record get "127.0.0.1 foo" =
    { "1" { "ipaddr" = "127.0.0.1" } 
          { "canonical" = "foo" }
          { "aliases" } }

  test Hosts.lns get two_entries =
   { "1" { "ipaddr" = "127.0.0.1" } 
          { "canonical" = "foo" }
          { "aliases" { "1" = "foo.example.com" } }
    }
    { }
    { "2" { "ipaddr" = "192.168.0.1" } 
          { "canonical" = "pigiron.example.com" }
          { "aliases"
             { "1" = "pigiron" }
             { "2" = "pigiron.example" }  }  }

  test Hosts.record put "127.0.0.1 foo" after
      set "1/canonical" "bar" 
  = "127.0.0.1 bar"

  test Hosts.lns put two_entries after 
    set "2/aliases/10" "piggy" ;
    rm "1/aliases/1" ;
    rm "2/aliases/2" 
  = "127.0.0.1 foo
# comment
192.168.0.1 pigiron.example.com pigiron piggy
"

  (* Deleting the 'aliases' node violates the schema; each host entry *)
  (* must have one, even if it has no aliases                         *)
  test Hosts.lns put two_entries after
      rm "1/aliases"
    = *

(* Local Variables: *)
(* mode: caml *)
(* End: *)


