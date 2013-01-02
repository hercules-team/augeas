module Test_Networks =

let conf = "# Sample networks
default         0.0.0.0         # default route    - mandatory
loopnet         127.0.0.0    loopnet_alias  loopnet_alias2   # loopback network - mandatory
mynet           128.253.154   # Modify for your own network address

loopback        127
arpanet         10      arpa    # Historical
localnet        192.168.25.192
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
  {}
  { "4"
    { "name" = "loopback" }
    { "number" = "127" }
  }
  { "5"
    { "name" = "arpanet" }
    { "number" = "10" }
    { "aliases"
      { "1" = "arpa" }
    }
    { "#comment" = "Historical" }
  }
  { "6"
    { "name" = "localnet" }
    { "number" = "192.168.25.192" }
  }
