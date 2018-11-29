(*
Module: Test_Semanage
  Provides unit tests and examples for the <Semanage> lens.
*)

module Test_Semanage =

(* Variable: phony_conf *)
let phony_conf = "# this is a comment

mykey = myvalue # eol comment
anotherkey = another value
"

(* Test: Semanage.lns *)
test Semanage.lns get phony_conf =
   { "#comment" = "this is a comment" }
   { }
   { "mykey" = "myvalue"
     { "#comment" = "eol comment" } }
   { "anotherkey" = "another value" }

(* Test: Semanage.lns
   Quotes are OK in variables that do not begin with a quote *)
test Semanage.lns get "UserParameter=custom.vfs.dev.read.ops[*],cat /proc/diskstats | grep $1 | head -1 | awk '{print $$4}'\n" =
     { "UserParameter" = "custom.vfs.dev.read.ops[*],cat /proc/diskstats | grep $1 | head -1 | awk '{print $$4}'" }

(* Test: Semanage.lns
     Support empty values *)
test Semanage.lns get "foo =\n" =
  { "foo" }

(* Variable: conf *)
let conf = "module-store = direct
module-store = \"source\"

#policy-version = 19

expand-check=0

usepasswd=False
bzip-small=true
bzip-blocksize=5
ignoredirs=/root

[sefcontext_compile]
path = /usr/sbin/sefcontext_compile
args = -r $@

[end]

config=test

[verify module]
test=value
[end]
"

(* Test: Semanage.lns *)
test Semanage.lns get conf =
   { "module-store" = "direct" }
   { "module-store" = "source" }
   { }
   { "#comment" = "policy-version = 19" }
   { }
   { "expand-check" = "0" }
   { }
   { "usepasswd" = "False" }
   { "bzip-small" = "true" }
   { "bzip-blocksize" = "5" }
   { "ignoredirs" = "/root" }
   { }
   { "@group" = "sefcontext_compile"
     { "path" = "/usr/sbin/sefcontext_compile" }
     { "args" = "-r $@" }
     { } }
   { }
   { "config" = "test" }
   { }
   { "@group" = "verify module"
     { "test" = "value" } }
