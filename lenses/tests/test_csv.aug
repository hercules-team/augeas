module Test_CSV =

(* Test: CSV.lns
     Simple test *)
test CSV.lns get "a,b,c\n" =
  { "1"
    { "1" = "a" }
    { "2" = "b" }
    { "3" = "c" } }

(* Test: CSV.lns
     Values with spaces *)
test CSV.lns get "a,b c,d\n" =
  { "1"
    { "1" = "a" }
    { "2" = "b c" }
    { "3" = "d" } }

(* Test: CSV.lns
     Quoted values *)
test CSV.lns get "a,\"b,c\",d
# comment
#
e,f,with space\n" =
  { "1"
    { "1" = "a" }
    { "2" = "\"b,c\"" }
    { "3" = "d" } }
  { "#comment" = "comment" }
  { }
  { "2"
    { "1" = "e" }
    { "2" = "f" }
    { "3" = "with space" } }

(* Test: CSV.lns
     Empty values *)
test CSV.lns get ",
,,\n" =
  { "1"
    { "1" = "" }
    { "2" = "" } }
  { "2"
    { "1" = "" }
    { "2" = "" }
    { "3" = "" } }

(* Test: CSV.lns
     Trailing spaces *)
test CSV.lns get "a , b 
 \n" =
  { "1"
    { "1" = "a " }
    { "2" = " b " } }
  { "2"
    { "1" = " " } }

(* Test: CSV.lns
     Quoted values in quoted values *)
test CSV.lns get "\"a,b\"\"c d\"\"\"\n" =
  { "1" { "1" = "\"a,b\"\"c d\"\"\"" } }

(* Test: CSV.lns
     Quote in quoted values *)
test CSV.lns get "\"a,b\"\"c d\"\n" =
  { "1" { "1" = "\"a,b\"\"c d\"" } }

(* Test: CSV.lns
     Values with newlines *)
test CSV.lns get "a,\"b\n c\"\n" =
  { "1"
    { "1" = "a" }
    { "2" = "\"b\n c\"" } }

(* Test: CSV.lns_semicol
     Semi-colon lens *)
test CSV.lns_semicol get "a;\"b;c\";d
# comment
#
e;f;with space\n" =
  { "1"
    { "1" = "a" }
    { "2" = "\"b;c\"" }
    { "3" = "d" } }
  { "#comment" = "comment" }
  {  }
  { "2"
    { "1" = "e" }
    { "2" = "f" }
    { "3" = "with space" } }

