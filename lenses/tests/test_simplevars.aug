(*
Module: Test_Simplevars
  Provides unit tests and examples for the <Simplevars> lens.
*)

module Test_Simplevars =

(* Variable: conf *)
let conf = "# this is a comment

mykey = myvalue # eol comment
anotherkey = another value
"

(* Test: Simplevars.lns *)
test Simplevars.lns get conf =
   { "#comment" = "this is a comment" }
   { }
   { "mykey" = "myvalue"
     { "#comment" = "eol comment" } }
   { "anotherkey" = "another value" }

(* Test: Simplevars.lns
   Quotes are OK in variables that do not begin with a quote *)
test Simplevars.lns get "UserParameter=custom.vfs.dev.read.ops[*],cat /proc/diskstats | grep $1 | head -1 | awk '{print $$4}'\n" =
     { "UserParameter" = "custom.vfs.dev.read.ops[*],cat /proc/diskstats | grep $1 | head -1 | awk '{print $$4}'" }

(* Test: Simplevars.lns
    Support flags *)
test Simplevars.lns get "dnsadminapp\n" =
  { "dnsadminapp" }

(* Test: Simplevars.lns
     Support empty values *)
test Simplevars.lns get "foo =\n" =
  { "foo" = "" { } }

(* Variable: bsd_adduser
     adduser(8) on the BSDs writes its config as perl, not as shell *)
let bsd_adduser = "# verbose = [0-2]
verbose = 1

# copy dotfiles from this dir (\"/etc/skel\" or \"no\")
dotdir = \"/etc/skel\"

# List of directories where shells located
# path = ('/bin', '/usr/bin', '/usr/local/bin')
path = ('/bin', '/usr/bin', '/usr/local/bin')

## DO NOT DELETE THIS LINE!
"

(* Test: Simplevars.lns *)
test Simplevars.lns get bsd_adduser =
   { "#comment" = "verbose = [0-2]" }
   { "verbose" = "1" }
   { }
   { "#comment" = "copy dotfiles from this dir (\"/etc/skel\" or \"no\")" }
   { "dotdir" = "\"/etc/skel\"" }
   { }
   { "#comment" = "List of directories where shells located" }
   { "#comment" = "path = ('/bin', '/usr/bin', '/usr/local/bin')" }
   { "path" = "('/bin', '/usr/bin', '/usr/local/bin')" }
   { }
   { "#comment" = "# DO NOT DELETE THIS LINE!" }
