module Test_automounter =

  let example = "#
# This is an automounter map and it has the following format
# key [ -mount-options-separated-by-comma ] location
# Details may be found in the autofs(5) manpage

# indirect map
cd      -fstype=iso9660,ro,nosuid,nodev :/dev/cdrom
kernel    -ro,soft,intr       ftp.kernel.org:/pub/linux
*       -fstype=auto,loop,ro    :/srv/distros/isos/&.iso

# direct map
/nfs/apps/mozilla             bogus:/usr/local/moxill

# replicated server
path    host1,host2,hostn:/path/path
path    host1,host2:/blah host3(1):/some/other/path
path    host1(5),host2(6),host3(1):/path/path

# multi-mount map
server    -rw,hard,intr       / -ro myserver.me.org:/
server    -rw,hard,intr       / -ro myserver.me.org:/ /usr myserver.me.org:/usr
server    -rw,hard,intr       / -ro myserver.me.org:/ \
                              /usr myserver.me.org:/usr \
                              /home myserver.me.org:/home

server    -rw,hard,intr       / -ro my-with-dash-server.me.org:/

# included maps
+auto_home
"

  test Automounter.lns get example =
    { }
    { "#comment" = "This is an automounter map and it has the following format" }
    { "#comment" = "key [ -mount-options-separated-by-comma ] location" }
    { "#comment" = "Details may be found in the autofs(5) manpage" }
    { }
    { "#comment" = "indirect map" }
    { "1" = "cd"
        { "opt" = "fstype"
            { "value" = "iso9660" } }
        { "opt" = "ro" }
        { "opt" = "nosuid" }
        { "opt" = "nodev" }
        { "location"
            { "1"
                { "path" = "/dev/cdrom" } } } }
    { "2" = "kernel"
        { "opt" = "ro" }
        { "opt" = "soft" }
        { "opt" = "intr" }
        { "location"
            { "1"
                { "host" = "ftp.kernel.org" }
                { "path" = "/pub/linux" } } } }
    { "3" = "*"
        { "opt" = "fstype"
            { "value" = "auto" } }
        { "opt" = "loop" }
        { "opt" = "ro" }
        { "location"
            { "1"
                { "path" = "/srv/distros/isos/&.iso" } } } }
    { }
    { "#comment" = "direct map" }
    { "4" = "/nfs/apps/mozilla"
        { "location"
            { "1"
                { "host" = "bogus" }
                { "path" = "/usr/local/moxill" } } } }
    { }
    { "#comment" = "replicated server" }
    { "5" = "path"
        { "location"
            { "1"
                { "host" = "host1" }
                { "host" = "host2" }
                { "host" = "hostn" }
                { "path" = "/path/path" } } } }
    { "6" = "path"
        { "location"
            { "1"
                { "host" = "host1" }
                { "host" = "host2" }
                { "path" = "/blah" } }
            { "2"
                { "host" = "host3"
                    { "weight" = "1" } }
                { "path" = "/some/other/path" } } } }
    { "7" = "path"
        { "location"
            { "1"
                { "host" = "host1"
                    { "weight" = "5" } }
                { "host" = "host2"
                    { "weight" = "6" } }
                { "host" = "host3"
                    { "weight" = "1" } }
                { "path" = "/path/path" } } } }
    { }
    { "#comment" = "multi-mount map" }
    { "8" = "server"
        { "opt" = "rw" }
        { "opt" = "hard" }
        { "opt" = "intr" }
        { "mount"
            { "1" = "/"
                { "opt" = "ro" }
                { "location"
                    { "1"
                        { "host" = "myserver.me.org" }
                        { "path" = "/" } } } } } }
    { "9" = "server"
        { "opt" = "rw" }
        { "opt" = "hard" }
        { "opt" = "intr" }
        { "mount"
            { "1" = "/"
                { "opt" = "ro" }
                { "location"
                    { "1"
                        { "host" = "myserver.me.org" }
                        { "path" = "/" } } } }
            { "2" = "/usr"
                { "location"
                    { "1"
                        { "host" = "myserver.me.org" }
                        { "path" = "/usr" } } } } } }
    { "10" = "server"
        { "opt" = "rw" }
        { "opt" = "hard" }
        { "opt" = "intr" }
        { "mount"
            { "1" = "/"
                { "opt" = "ro" }
                { "location"
                    { "1"
                        { "host" = "myserver.me.org" }
                        { "path" = "/" } } } }
            { "2" = "/usr"
                { "location"
                    { "1"
                        { "host" = "myserver.me.org" }
                        { "path" = "/usr" } } } }
            { "3" = "/home"
                { "location"
                    { "1"
                        { "host" = "myserver.me.org" }
                        { "path" = "/home" } } } } } }
    { }
    { "11" = "server"
        { "opt" = "rw" }
        { "opt" = "hard" }
        { "opt" = "intr" }
        { "mount"
            { "1" = "/"
                { "opt" = "ro" }
                { "location"
                    { "1"
                        { "host" = "my-with-dash-server.me.org" }
                        { "path" = "/" } } } } } }
    { }
    { "#comment" = "included maps" }
    { "12" = "+"
        { "map" = "auto_home" } }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
