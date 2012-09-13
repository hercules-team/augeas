(*
Module: Test_Collectd
  Provides unit tests and examples for the <Collectd> lens.
*)
module Test_Collectd =

(* Variable: simple *)
let simple = "LoadPlugin contextswitch
LoadPlugin cpu
FQDNLookup \"true\"
Include \"/var/lib/puppet/modules/collectd/plugins/*.conf\"
"

(* Test: Collectd.lns *)
test Collectd.lns get simple =
  { "directive" = "LoadPlugin"
    { "arg" = "contextswitch" }
  }
  { "directive" = "LoadPlugin"
    { "arg" = "cpu" }
  }
  { "directive" = "FQDNLookup"
    { "arg" = "\"true\"" }
  }
  { "directive" = "Include"
    { "arg" = "\"/var/lib/puppet/modules/collectd/plugins/*.conf\"" }
  }


(* Variable: filters *)
let filters = "<Chain \"PreCache\">
       <Rule \"no_fqdn\">
               <Match \"regex\">
                       Host \"^[^\.]*$\"
                       Invert false
               </Match>
               Target \"stop\"
       </Rule>
</Chain>
"


(* Test: Collectd.lns *)
test Collectd.lns get filters =
  { "Chain"
    { "arg" = "\"PreCache\"" }
    { "Rule"
      { "arg" = "\"no_fqdn\"" }
      { "Match"
        { "arg" = "\"regex\"" }
        { "directive" = "Host"
          { "arg" = "\"^[^\.]*$\"" }
        }
        { "directive" = "Invert"
          { "arg" = "false" }
        }
      }
      { "directive" = "Target"
        { "arg" = "\"stop\"" }
      }
    }
  }



