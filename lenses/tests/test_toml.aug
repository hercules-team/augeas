module Test_Toml =

(* Test: Toml.lns
     Global parameters *)
test Toml.lns get "# Globals
foo = \"bar\"\n" =
  { "#comment" = "Globals" }
  { "foo" = "bar" }

(* Test: Toml.lns
     Simple section/value *)
test Toml.lns get "[foo]
bar = \"baz\"\n" =
  { "@table" = "foo" { "bar" = "baz" } }

(* Test: Toml.lns
     Subsections *)
test Toml.lns get "[foo]
title = bar
  [foo.one]
  hello = world\n" =
  { "@table" = "foo"
    { "title" = "bar" } }
  { "@table" = "foo.one"
    { "hello" = "world" } }

(* Test: Toml.lns
     Nested subsections *)
test Toml.lns get "[foo]
[foo.one]
[foo.one.two]
bar = baz\n" =
  { "@table" = "foo" }
  { "@table" = "foo.one" }
  { "@table" = "foo.one.two"
    { "bar" = "baz" } }

(* Test: Toml.lns
     Integer type *)
test Toml.lns get "foo = 42
bar = -17\n" =
  { "foo" = "42" }
  { "bar" = "-17" }

(* Test: Toml.lns
     Float type *)
test Toml.lns get "foo = 3.1415
bar = -0.01\n" =
  { "foo" = "3.1415" }
  { "bar" = "-0.01" }

(* Test: Toml.lns
     Boolean type *)
test Toml.lns get "foo = true
bar = false\n" =
  { "foo" = "true" }
  { "bar" = "false" }

(* Test: Toml.lns
     Datetime type *)
test Toml.lns get "foo = 1979-05-27T07:32:00Z\n" =
  { "foo" = "1979-05-27T07:32:00Z" }

(* Test: Toml.lns
    Array values *)
test Toml.lns get "foo = [ \"bar\", 2 ]\n" =
  { "foo"
    { "elem" = "bar" }
    { "elem" = "2" } }

(* Test: Toml.lns
     Nested arrays *)
test Toml.lns get "foo = [ \"bar\",
  [ 1, 2, [\"hello\"]], \"baz\"
]\n" =
  { "foo"
    { "elem" = "bar" }
    { "elem" { "elem" = "1" }
          { "elem" = "2" }
          { "elem" { "elem" = "hello" } } }
    { "elem" = "baz" } }

(* Test: Toml.lns
     Arrays of tables *)
test Toml.lns get "[[products]]
name = \"Hammer\"
sku = 738594937

[[products]]

[[products]]
name = \"Nail\"
sku = 284758393
color = \"gray\"\n" =
  { "@@table" = "products"
    { "name" = "Hammer" }
    { "sku" = "738594937" }
    { } }
  { "@@table" = "products" { } }
  { "@@table" = "products"
    { "name" = "Nail" }
    { "sku" = "284758393" }
    { "color" = "gray" } }

(* Test: Toml.lns
     Multiline values *)
test Toml.lns get "foo = \"bar\nbaz\"\n" =
  { "foo" = "bar\nbaz" }

(* Variable: example
     The example from https://github.com/mojombo/toml *)
let example = "# This is a TOML document. Boom.

title = \"TOML Example\"

[owner]
name = \"Tom Preston-Werner\"
organization = \"GitHub\"
bio = \"GitHub Cofounder & CEO\nLikes tater tots and beer.\"
dob = 1979-05-27T07:32:00Z # First class dates? Why not?

[database]
server = \"192.168.1.1\"
ports = [ 8001, 8001, 8002 ]
connection_max = 5000
enabled = true

[servers]

  # You can indent as you please. Tabs or spaces. TOML don't care.
  [servers.alpha]
  ip = \"10.0.0.1\"
  dc = \"eqdc10\"

  [servers.beta]
  ip = \"10.0.0.2\"
  dc = \"eqdc10\"

[clients]
data = [ [\"gamma\", \"delta\"], [1, 2] ]

# Line breaks are OK when inside arrays
hosts = [
  \"alpha\",
  \"omega\"
]\n"


(* Test: Toml.lns
     Parse <example> *)
test Toml.lns get example = 
  { "#comment" = "This is a TOML document. Boom." }
  { }
  { "title" = "TOML Example" }
  { }
  { "@table" = "owner"
    { "name" = "Tom Preston-Werner" }
    { "organization" = "GitHub" }
    { "bio" = "GitHub Cofounder & CEO\nLikes tater tots and beer." }
    { "dob" = "1979-05-27T07:32:00Z" { "#comment" = "First class dates? Why not?" } }
    { } }
  { "@table" = "database"
    { "server" = "192.168.1.1" }
    { "ports" { "elem" = "8001" } { "elem" = "8001" } { "elem" = "8002" } }
    { "connection_max" = "5000" }
    { "enabled" = "true" }
    { } }
  { "@table" = "servers"
    { }
    { "#comment" = "You can indent as you please. Tabs or spaces. TOML don't care." } }
  { "@table" = "servers.alpha"
    { "ip" = "10.0.0.1" }
    { "dc" = "eqdc10" }
    { } }
  { "@table" = "servers.beta"
    { "ip" = "10.0.0.2" }
    { "dc" = "eqdc10" }
  { } }
  { "@table" = "clients"
    { "data"
      { "elem" { "elem" = "gamma" } { "elem" = "delta" } }
      { "elem" { "elem" = "1" } { "elem" = "2" } } }
    { }
    { "#comment" = "Line breaks are OK when inside arrays" }
    { "hosts" { "elem" = "alpha" } { "elem" = "omega" } } }


