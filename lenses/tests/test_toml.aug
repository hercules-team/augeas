module Test_Toml =

(* Test: Toml.norec
     String value *)
test Toml.norec get "\"foo\"" = { "string" = "foo" }

(* Test: Toml.norec
     Integer value *)
test Toml.norec get "42" = { "integer" = "42" }

(* Test: Toml.norec
     Positive integer value *)
test Toml.norec get "+42" = { "integer" = "+42" }

(* Test: Toml.norec
     Negative integer value *)
test Toml.norec get "-42" = { "integer" = "-42" }

(* Test: Toml.norec
     Large integer value *)
test Toml.norec get "5_349_221" = { "integer" = "5_349_221" }

(* Test: Toml.norec
     Hexadecimal integer value *)
test Toml.norec get "0xDEADBEEF" = { "integer" = "0xDEADBEEF" }

(* Test: Toml.norec
     Octal integer value *)
test Toml.norec get "0o755" = { "integer" = "0o755" }

(* Test: Toml.norec
     Binary integer value *)
test Toml.norec get "0b11010110" = { "integer" = "0b11010110" }

(* Test: Toml.norec
     Float value *)
test Toml.norec get "3.14" = { "float" = "3.14" }

(* Test: Toml.norec
     Positive float value *)
test Toml.norec get "+3.14" = { "float" = "+3.14" }

(* Test: Toml.norec
     Negative float value *)
test Toml.norec get "-3.14" = { "float" = "-3.14" }

(* Test: Toml.norec
     Complex float value *)
test Toml.norec get "-3_220.145_223e-34" = { "float" = "-3_220.145_223e-34" }

(* Test: Toml.norec
     Inf float value *)
test Toml.norec get "-inf" = { "float" = "-inf" }

(* Test: Toml.norec
     Nan float value *)
test Toml.norec get "-nan" = { "float" = "-nan" }

(* Test: Toml.norec
     Bool value *)
test Toml.norec get "true" = { "bool" = "true" }

(* Test: Toml.norec
     Datetime value *)
test Toml.norec get "1979-05-27T07:32:00Z" =
  { "datetime" = "1979-05-27T07:32:00Z" }
test Toml.norec get "1979-05-27 07:32:00.999999" =
  { "datetime" = "1979-05-27 07:32:00.999999" }

(* Test: Toml.norec 
     Date value *)
test Toml.norec get "1979-05-27" =
  { "date" = "1979-05-27" }

(* Test: Toml.norec 
     Time value *)
test Toml.norec get "07:32:00" =
  { "time" = "07:32:00" }

(* Test: Toml.norec
     String value with newline *)
test Toml.norec get "\"bar\nbaz\"" =
  { "string" = "bar\nbaz" }

(* Test: Toml.norec
     Multiline value *)
test Toml.norec get "\"\"\"\nbar\nbaz\n    \"\"\"" =
  { "string_multi" = "bar\nbaz" }

(* Test: Toml.norec
     Literal string value *)
test Toml.norec get "'bar\nbaz'" =
  { "string_literal" = "bar\nbaz" }

(* Test: Toml.array_norec
     Empty array *)
test Toml.array_norec get "[ ]" =
  { "array" {} }

(* Test: Toml.array_norec
     Array of strings *)
test Toml.array_norec get "[ \"foo\", \"bar\" ]" =
  { "array" {}
    { "string" = "foo" } {}
    { "string" = "bar" } {} }

(* Test: Toml.array_norec
     Array of arrays *)
test Toml.array_rec get "[ [ \"foo\", [ 42, 43 ] ], \"bar\" ]" =
  { "array" {}
    { "array" {}
      { "string" = "foo" } {}
      { "array" {}
        { "integer" = "42" } {}
        { "integer" = "43" } {} } {} } {}
    { "string" = "bar" } {} }

(* Test: Toml.lns
     Global parameters *)
test Toml.lns get "# Globals
foo = \"bar\"\n" =
  { "#comment" = "Globals" }
  { "entry" = "foo" { "string" = "bar" } }

(* Test: Toml.lns
     Simple section/value *)
test Toml.lns get "[foo]
bar = \"baz\"\n" =
  { "table" = "foo" { "entry" = "bar" { "string" = "baz" } } }

(* Test: Toml.lns
     Subsections *)
test Toml.lns get "[foo]
title = \"bar\"
  [foo.one]
  hello = \"world\"\n" =
  { "table" = "foo"
    { "entry" = "title" { "string" = "bar" } } }
  { "table" = "foo.one"
    { "entry" = "hello" { "string" = "world" } } }

(* Test: Toml.lns
     Nested subsections *)
test Toml.lns get "[foo]
[foo.one]
[foo.one.two]
bar = \"baz\"\n" =
  { "table" = "foo" }
  { "table" = "foo.one" }
  { "table" = "foo.one.two"
    { "entry" = "bar" { "string" = "baz" } } }

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
  { "@table" = "products"
    { "entry" = "name"
      { "string" = "Hammer" } }
    { "entry" = "sku"
      { "integer" = "738594937" } }
    {  }
  }
  { "@table" = "products"
    {  }
  }
  { "@table" = "products"
    { "entry" = "name"
      { "string" = "Nail" } }
    { "entry" = "sku"
      { "integer" = "284758393" } }
    { "entry" = "color"
      { "string" = "gray" } }
  }

(* Test: Toml.entry
     Empty inline table *)
test Toml.entry get "name = { }\n" =
  { "entry" = "name"
    { "inline_table"
      {  } } }

(* Test: Toml.entry
     Inline table *)
test Toml.entry get "name = { first = \"Tom\", last = \"Preston-Werner\" }\n" =
  { "entry" = "name"
    { "inline_table" {}
      { "entry" = "first"
        { "string" = "Tom"  } } {}
      { "entry" = "last"
        { "string" = "Preston-Werner"  } } {} } }

(* Test: Toml.entry
    Array value in inline_table *)
test Toml.entry get "foo = { bar = [\"baz\"] }\n" =
  { "entry" = "foo"
    { "inline_table" {}
      { "entry" = "bar"
        { "array"
          { "string" = "baz" } } } {} } }


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
  country = \"中国\" # This should be parsed as UTF-8

[clients]
data = [ [\"gamma\", \"delta\"], [1, 2] ] # just an update to make sure parsers support it

# Line breaks are OK when inside arrays
hosts = [
  \"alpha\",
  \"omega\"
]

# Products

  [[products]]
  name = \"Hammer\"
  sku = 738594937

  [[products]]
  name = \"Nail\"
  sku = 284758393
  color = \"gray\"
"

test Toml.lns get example =
  { "#comment" = "This is a TOML document. Boom." } {  }
  { "entry" = "title"
    { "string" = "TOML Example" } } {  }
  { "table" = "owner"
    { "entry" = "name"
      { "string" = "Tom Preston-Werner" } }
    { "entry" = "organization"
      { "string" = "GitHub" } }
    { "entry" = "bio"
      { "string" = "GitHub Cofounder & CEO
Likes tater tots and beer." }
    }
    { "entry" = "dob"
      { "datetime" = "1979-05-27T07:32:00Z" }
      { "#comment" = "First class dates? Why not?" } } {  } }
  { "table" = "database"
    { "entry" = "server"
      { "string" = "192.168.1.1" } }
    { "entry" = "ports"
      { "array" {  }
        { "integer" = "8001" } {  }
        { "integer" = "8001" } {  }
        { "integer" = "8002" } {  } } }
    { "entry" = "connection_max"
      { "integer" = "5000" } }
    { "entry" = "enabled"
      { "bool" = "true" } } {  } }
  { "table" = "servers" {  }
    { "#comment" = "You can indent as you please. Tabs or spaces. TOML don't care." } }
  { "table" = "servers.alpha"
    { "entry" = "ip"
      { "string" = "10.0.0.1" } }
    { "entry" = "dc"
      { "string" = "eqdc10" } } {  } }
  { "table" = "servers.beta"
    { "entry" = "ip"
      { "string" = "10.0.0.2" } }
    { "entry" = "dc"
      { "string" = "eqdc10" } }
    { "entry" = "country"
      { "string" = "中国" }
      { "#comment" = "This should be parsed as UTF-8" } } {  } }
  { "table" = "clients"
    { "entry" = "data"
      { "array" {  }
        { "array"
          { "string" = "gamma" } {  }
          { "string" = "delta" } } {  }
        { "array"
          { "integer" = "1" } {  }
          { "integer" = "2" } } {  } }
      { "#comment" = "just an update to make sure parsers support it" } } {  }
    { "#comment" = "Line breaks are OK when inside arrays" }
    { "entry" = "hosts"
      { "array" {  }
        { "string" = "alpha" } {  }
        { "string" = "omega" } {  } } } {  }
    { "#comment" = "Products" } {  } }
  { "@table" = "products"
    { "entry" = "name"
      { "string" = "Hammer" } }
    { "entry" = "sku"
      { "integer" = "738594937" } } {  } }
  { "@table" = "products"
    { "entry" = "name"
      { "string" = "Nail" } }
    { "entry" = "sku"
      { "integer" = "284758393" } }
    { "entry" = "color"
      { "string" = "gray" } } }

