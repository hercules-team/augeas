module Test_Reprepro_Uploaders =

(* Star condition *)
test Reprepro_Uploaders.entry get "allow * by anybody\n" =
  { "allow" = "*"
    { "by" = "anybody" } }

(* Simple key *)
test Reprepro_Uploaders.entry get "allow * by key ABCD1234\n" =
  { "allow" = "*"
    { "by" = "key"
      { "key" = "ABCD1234" } } }

(* Simple condition *)
test Reprepro_Uploaders.entry get "allow source 'bash' by anybody\n" =
  { "allow"
    { "and" { "or" { "source" = "bash" } } }
    { "by" = "anybody" } }

(* Simple 'and' *)
test Reprepro_Uploaders.entry get "allow source 'bash' and binaries contain 'bash-doc' by anybody\n" =
  { "allow"
    { "and" { "or" { "source" = "bash" } } }
    { "and" { "or" { "binaries contain" = "bash-doc" } } }
    { "by" = "anybody" } }

(* Simple 'or' *)
test Reprepro_Uploaders.entry get "allow source 'bash' or binaries contain 'bash-doc' by anybody\n" =
  { "allow"
    { "and" { "or" { "source" = "bash" } }
            { "or" { "binaries contain" = "bash-doc" } } }
    { "by" = "anybody" } }

(* 'and' + 'or' *)
test Reprepro_Uploaders.entry get "allow sections 'main|restricted' and source 'bash' or binaries contain 'bash-doc' by anybody\n" =
  { "allow"
    { "and" { "or" { "sections" = "main|restricted" } } }
    { "and" { "or" { "source" = "bash" } }
            { "or" { "binaries contain" = "bash-doc" } } }
    { "by" = "anybody" } }


let conf = "# ftpmaster
allow * by key 74BF771E

allow sections 'desktop/*' by anybody
allow sections 'gforge/*' and binaries contain 'bzr' or source '*melanie*' by any key
"

test Reprepro_Uploaders.lns get conf =
  { "#comment" = "ftpmaster" }
  { "allow" = "*"
    { "by" = "key"
      { "key" = "74BF771E" } } }
  { }
  { "allow"
    { "and" { "or" { "sections" = "desktop/*" } } }
    { "by" = "anybody" } }
  { "allow"
    { "and" { "or" { "sections" = "gforge/*" } } }
    { "and" { "or" { "binaries contain" = "bzr" } }
            { "or" { "source" = "*melanie*" } } }
    { "by" = "key"
      { "key" = "any" } } }
