(* Parsing /etc/dpkg/dpkg.cfg *)

(*

dpkg.cfg is a simple list of options, the same ones as the command
line options, with or without a value.

The tree is a list of either comments or option/value pairs by name.  Use "set"
to set an option with a value, and "clear" for a bare option.

Example:

$ augtool -n
augtool> ls /files/etc/dpkg/dpkg.cfg
#comment[1] = dpkg configuration file
#comment[2] = This file can contain default options for dpkg.  All command-line
#comment[3] = options are allowed.  Values can be specified by putting them after
#comment[4] = the option, separated by whitespace and/or an `=' sign.
#comment[5] = Do not enable debsig-verify by default; since the distribution is not using
#comment[6] = embedded signatures, debsig-verify would reject all packages.
no-debsig = (none)
#comment[7] = Log status changes and actions to a file.
log = /var/log/dpkg.log
augtool> get /files/etc/dpkg/dpkg.cfg/no-debsig
/files/etc/dpkg/dpkg.cfg/no-debsig (none)
augtool> get /files/etc/dpkg/dpkg.cfg/log
/files/etc/dpkg/dpkg.cfg/log = /var/log/dpkg.log
augtool> clear /files/etc/dpkg/dpkg.cfg/testopt
augtool> set /files/etc/dpkg/dpkg.cfg/testopt2 test
augtool> save
Saved 1 file(s)
augtool>
domU-12-31-39-03-40-A6:/var/tmp# cat /etc/dpkg/dpkg.cfg.augnew
# dpkg configuration file
#
# This file can contain default options for dpkg.  All command-line
# options are allowed.  Values can be specified by putting them after
# the option, separated by whitespace and/or an `=' sign.
#

# Do not enable debsig-verify by default; since the distribution is not using
# embedded signatures, debsig-verify would reject all packages.
no-debsig

# Log status changes and actions to a file.
log /var/log/dpkg.log
testopt
testopt2 test

*)

module Dpkg =
  autoload xfm

  let sep_tab = Util.del_ws_tab
  let sep_spc = Util.del_ws_spc
  let eol = del /[ \t]*\n/ "\n"

  let comment = Util.comment
  let empty   = Util.empty

  let word = /[^,# \n\t]+/
  let keyword = /[^,# \n\t\/]+/

  let record = [ key keyword . (sep_spc . store word)? . eol ]

  let lns = ( empty | comment | record ) *

  let xfm = transform lns (incl "/etc/dpkg/dpkg.cfg")

(* Local Variables: *)
(* mode: caml *)
(* End: *)
