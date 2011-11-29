(*
Module: Test_Reprepro_Uploaders
  Provides unit tests and examples for the <Reprepro_Uploaders> lens.
*)

module Test_Reprepro_Uploaders =

(* View: Reprepro_Uploaders.entry
     A star condition gets mapped as direct value
     of the "allow" node.

     "anybody" and "unsigned" get mapped as direct
     values of the "by" subnode. *)
test Reprepro_Uploaders.entry get "allow * by anybody\n" =
  { "allow" = "*"
    { "by" = "anybody" } }

(* View: Reprepro_Uploaders.entry
     For simple keys, the "by" node gets the value "key"
     and the key ID gets mapped in a "key" subnode. *)
test Reprepro_Uploaders.entry get "allow * by key ABCD1234\n" =
  { "allow" = "*"
    { "by" = "key"
      { "key" = "ABCD1234" } } }

(* View: Reprepro_Uploaders.entry
     Conditions are mapped inside a tree containing
     at least an "and" node and an "or" subnode.

     The value of each "or" subnode is the type of check
     (e.g. "source"), and this node contains "or" subnodes
     with the value(s) allowed for the check (e.g. "bash"). *)
test Reprepro_Uploaders.entry get "allow source 'bash' by anybody\n" =
  { "allow"
    { "and" { "or" = "source" { "or" = "bash" } } }
    { "by" = "anybody" } }

(* Simple 'and' *)
test Reprepro_Uploaders.entry get "allow source 'bash' and binaries contain 'bash-doc' by anybody\n" =
  { "allow"
    { "and" { "or" = "source" { "or" = "bash" } } }
    { "and" { "or" = "binaries" { "contain" } { "or" = "bash-doc" } } }
    { "by" = "anybody" } }

(* Simple 'or' *)
test Reprepro_Uploaders.entry get "allow source 'bash' or binaries contain 'bash-doc' by anybody\n" =
  { "allow"
    { "and" { "or" = "source" { "or" = "bash" } }
            { "or" = "binaries" { "contain" } { "or" = "bash-doc" } } }
    { "by" = "anybody" } }

(* 'and' + 'or' *)
test Reprepro_Uploaders.entry get "allow sections 'main'|'restricted' and source 'bash' or binaries contain 'bash-doc' by anybody\n" =
  { "allow"
    { "and" { "or" = "sections" { "or" =  "main" } { "or" = "restricted" } } }
    { "and" { "or" = "source" { "or" = "bash" } }
            { "or" = "binaries" { "contain" } { "or" = "bash-doc" } } }
    { "by" = "anybody" } }

(* Not *)
test Reprepro_Uploaders.entry get "allow not source 'bash' by anybody\n" =
  { "allow" { "and" { "or" = "source" { "not" } { "or" = "bash" } } }
    { "by" = "anybody" } }



let conf = "# ftpmaster
allow * by key 74BF771E

allow sections 'desktop/*' by anybody
allow sections 'gforge/*' and binaries contain 'bzr' or not source '*melanie*'|'katya' by any key
"

test Reprepro_Uploaders.lns get conf =
  { "#comment" = "ftpmaster" }
  { "allow" = "*"
    { "by" = "key"
      { "key" = "74BF771E" } } }
  { }
  { "allow"
    { "and" { "or" = "sections" { "or" = "desktop/*" } } }
    { "by" = "anybody" } }
  { "allow"
    { "and" { "or" = "sections" { "or" = "gforge/*" } } }
    { "and" { "or" = "binaries" { "contain" } { "or" = "bzr" } }
            { "or" = "source" { "not" } { "or" = "*melanie*" } { "or" = "katya" } } }
    { "by" = "key"
      { "key" = "any" } } }
