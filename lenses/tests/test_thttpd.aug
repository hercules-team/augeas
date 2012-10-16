(*
Module: Test_Thttpd
  Provides unit tests and examples for the <Thttpd> lens.
*)

module Test_Thttpd =

let conf = "# This file is for thttpd processes created by /etc/init.d/thttpd.
# Commentary is based closely on the thttpd(8) 2.25b manpage, by Jef Poskanzer.

# Specifies an alternate port number to listen on.
port=80
host=

  dir=/var/www
chroot
  novhost

  # Specifies what user to switch to after initialization when started as root.
user=www-data # EOL comment
nosymlinks # EOL comment
"

test Thttpd.lns get conf =
  { "#comment" = "This file is for thttpd processes created by /etc/init.d/thttpd." }
  { "#comment" = "Commentary is based closely on the thttpd(8) 2.25b manpage, by Jef Poskanzer." }
  { }
  { "#comment" = "Specifies an alternate port number to listen on." }
  { "port" = "80" }
  { "host" = "" }
  { }
  { "dir" = "/var/www" }
  { "chroot" }
  { "novhost" }
  { }
  { "#comment" = "Specifies what user to switch to after initialization when started as root." }
  { "user" = "www-data"
    { "#comment" = "EOL comment" }
  }
  { "nosymlinks"
    { "#comment" = "EOL comment" }
  }

(* There must not be spaces around the '=' *)
test Thttpd.lns get "port = 80" = *
