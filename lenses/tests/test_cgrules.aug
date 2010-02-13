module Test_cgrules =

let conf="#cgrules test configuration file
poooeter       cpu             test1/
%              memory          test2/
@somegroup     cpu             toto/
%              devices         toto1/
%              memory          toto3/
"
test Cgrules.lns get conf =
  { "#comment" = "cgrules test configuration file" }
  { "user" = "poooeter"
    { "cpu"    = "test1/" }
    { "memory" = "test2/" } }
  { "group" = "somegroup"
    { "cpu"     = "toto/" }
    { "devices" = "toto1/" }
    { "memory"  = "toto3/" } }

test Cgrules.lns put conf after
  set  "user/cpu" "test3/";
  rm   "user/memory";
  rm   "group";
  insa "devices" "user/*[last()]";
  set  "user/devices" "newtest/";
  insb "memory"  "user/devices";
  set  "user/memory"  "memtest/"
= "#cgrules test configuration file
poooeter       cpu             test3/
%              memory          memtest/
% devices newtest/
"
