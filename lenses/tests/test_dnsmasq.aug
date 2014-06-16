module Test_dnsmasq =

let conf = "# Configuration file for dnsmasq.
#
#bogus-priv

conf-dir=/etc/dnsmasq.d
selfmx

address=/foo.com/bar.net/10.1.2.3

server=10.4.5.6#1234
server=/bar.com/foo.net/10.7.8.9
server=/foo.org/bar.org/10.3.2.1@eth0#5678
server=/baz.org/#
server=/baz.net/#@eth1
server=10.6.5.4#1234@eth0#5678
server=/qux.com/qux.net/
"

test Dnsmasq.lns get conf =
  { "#comment" = "Configuration file for dnsmasq." }
  {}
  { "#comment" = "bogus-priv" }
  {}
  { "conf-dir" = "/etc/dnsmasq.d" }
  { "selfmx" }
  {}
  { "address" = "10.1.2.3"
    { "domain" = "foo.com" }
    { "domain" = "bar.net" }
  }
  {}
  { "server" = "10.4.5.6"
    { "port" = "1234" }
  }
  { "server" = "10.7.8.9"
    { "domain" = "bar.com" }
    { "domain" = "foo.net" }
  }
  { "server" = "10.3.2.1"
    { "domain" = "foo.org" }
    { "domain" = "bar.org" }
    { "source" = "eth0"
      { "port" = "5678" }
    }
  }
  { "server" = "#"
    { "domain" = "baz.org" }
  }
  { "server" = "#"
    { "domain" = "baz.net" }
    { "source" = "eth1" }
  }
  { "server" = "10.6.5.4"
    { "port" = "1234" }
    { "source" = "eth0"
      { "port" = "5678" }
    }
  }
  { "server"
    { "domain" = "qux.com" }
    { "domain" = "qux.net" }
  }
