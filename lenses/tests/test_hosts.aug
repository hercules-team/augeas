(* Tests for the Hosts module *)

module Test_hosts =

  let two_entries = "127.0.0.1 foo foo.example.com
# comment
192.168.0.1 pigiron.example.com pigiron pigiron.example
"

  test Hosts.record get "127.0.0.1 foo" =
    { "0" { "ipaddr" = "127.0.0.1" } 
          { "canonical" = "foo" }
          { "aliases" } }

  test Hosts.lns get two_entries =
   { "0" { "ipaddr" = "127.0.0.1" } 
          { "canonical" = "foo" }
          { "aliases" { "0" = "foo.example.com" } }
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

  test Hosts.lns put two_entries after 
    set "1/aliases/10" "piggy" ;
    rm "0/aliases/0" ;
    rm "1/aliases/1" 
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


