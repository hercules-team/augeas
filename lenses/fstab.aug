(* Parsing /etc/fstab *)

module Fstab =
  autoload xfm

  let sep_tab = Util.del_ws_tab
  let sep_spc = Util.del_ws_spc
  let comma = Util.del_str ","
  let eol = del /[ \t]*\n/ "\n"

  let comment = [ del /#.*\n/ "# " ]
  let word = /[^,# \n\t]+/
  let record = [ seq "mntent" . 
                   [ label "spec" . store  word ] . sep_tab .
                   [ label "file" . store word ] . sep_tab .
                   [ label "vfstype" . store word ] . sep_tab .
                   [ label "opt" . store word ] .
                     ([comma . label "opt" . store word])* .
                   (sep_tab . [ label "dump" . store /[0-9]+/ ] .
                    ( sep_spc . [ label "passno" . store /[0-9]+/ ])? )?
                 . eol ]

  let lns = ( comment | record ) *

  let xfm = transform lns (incl "/etc/fstab")

(* Local Variables: *)
(* mode: caml *)
(* End: *)


