(*
Module: Test_Approx
  Provides unit tests and examples for the <Approx> lens.
*)

module Test_approx =

(* Variable: default_approx
   A full configuration *)
  let default_approx = "# The following are the defaults, so there is no need
# to uncomment them unless you want a different value.
# See approx.conf(5) for details.

$interface	any
$port		9999
$interval	720
$max_wait	10
$max_rate	unlimited
$debug		false

# Here are some examples of remote repository mappings.
# See http://www.debian.org/mirror/list for mirror sites.

debian		http://ftp.nl.debian.org/debian
debian-volatile http://ftp.nl.debian.org/debian-volatile
security	http://security.debian.org
"

(* Test: Approx.lns
   Testing <Approx.lns> on <default_approx> *)
  test Approx.lns get default_approx =
  { "#comment" = "The following are the defaults, so there is no need" }
  { "#comment" = "to uncomment them unless you want a different value." }
  { "#comment" = "See approx.conf(5) for details." }
  {  }
  { "$interface" = "any" }
  { "$port" = "9999" }
  { "$interval" = "720" }
  { "$max_wait" = "10" }
  { "$max_rate" = "unlimited" }
  { "$debug" = "false" }
  {  }
  { "#comment" = "Here are some examples of remote repository mappings." }
  { "#comment" = "See http://www.debian.org/mirror/list for mirror sites." }
  {  }
  { "debian" = "http://ftp.nl.debian.org/debian" }
  { "debian-volatile" = "http://ftp.nl.debian.org/debian-volatile" }
  { "security" = "http://security.debian.org" }
