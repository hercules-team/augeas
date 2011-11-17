module Test_Networks =

let conf = "# Sample networks
default         0.0.0.0         # default route    - mandatory
loopnet         127.0.0.0    loopnet_alias  loopnet_alias2   # loopback network - mandatory
mynet           128.253.154   # Modify for your own network address
"

test Networks.lns get conf =
  { "#comment" = "Sample networks" }
  { "1"
    { "name" = "default" }
    { "number" = "0.0.0.0" }
    { "#comment" = "default route    - mandatory" }
  }
  { "2"
    { "name" = "loopnet" }
    { "number" = "127.0.0.0" }
    { "aliases"
      { "1" = "loopnet_alias" }
      { "2" = "loopnet_alias2" }
    }
    { "#comment" = "loopback network - mandatory" }
  }
  { "3"
    { "name" = "mynet" }
    { "number" = "128.253.154" }
    { "#comment" = "Modify for your own network address" }
  }
