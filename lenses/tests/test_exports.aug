module Test_exports =

  let s = "/local 172.31.0.0/16(rw,sync) \t

/home 172.31.0.0/16(rw,root_squash,sync) @netgroup(rw) *.example.com
# Yes, we export /tmp
/tmp 172.31.0.0/16(rw,root_squash,sync)\n"

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
            { "option" = "sync" } } }

