module Test_exports = 

let s = "/local 172.31.0.0/16(rw,sync) \t

/home 172.31.0.0/16(rw,root_squash,sync) @netgroup(rw) *.example.com
# Yes, we export /tmp
/tmp 172.31.0.0/16(rw,root_squash,sync,)
/local2 somehost(rw,sync)
/local3 some-host(rw,sync)
/local3 an-other-host(rw,sync)
/local4 2000:123:456::/64(rw)
"

test Exports.lns get s =
  { "dir" = "/local"
      { "client" = "172.31.0.0/16"
          { "option" = "rw" }
          { "option" = "sync" } } }
  { }
  { "dir" = "/home"
      { "client" = "172.31.0.0/16"
          { "option" = "rw"}
          { "option" = "root_squash" }
          { "option" = "sync" } }
      { "client" = "@netgroup"
          { "option" = "rw" } }
      { "client" = "*.example.com" } }
  { "#comment" = "Yes, we export /tmp" }
  { "dir" = "/tmp"
      { "client" = "172.31.0.0/16"
          { "option" = "rw" }
          { "option" = "root_squash" }
          { "option" = "sync" }
          { "option" = "" } } }
  { "dir" = "/local2"
      { "client" = "somehost"
          { "option" = "rw" }
          { "option" = "sync" } } }
  { "dir" = "/local3"
      { "client" = "some-host"
          { "option" = "rw" }
          { "option" = "sync" } } }
  { "dir" = "/local3"
      { "client" = "an-other-host"
          { "option" = "rw" }
          { "option" = "sync" } } }
  { "dir" = "/local4"
      { "client" = "2000:123:456::/64"
          { "option" = "rw" } } }
