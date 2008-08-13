(* Parsing /etc/fstab *)

module Fstab =
  autoload xfm

  let sep_tab = Util.del_ws_tab
  let sep_spc = Util.del_ws_spc
  let comma = Util.del_str ","
  let eol = del /[ \t]*\n/ "\n"

  let comment = Util.comment
  let empty   = Util.empty 

  let word = /[^,# \n\t]+/
  let comma_sep_list (l:string) =
    [ label l . store word ] . ([comma . label l . store word])*
  let record = [ seq "mntent" . 
                   [ label "spec" . store  word ] . sep_tab .
                   [ label "file" . store word ] . sep_tab .
                   comma_sep_list "vfstype" . sep_tab .
                   comma_sep_list "opt" .
                   (sep_tab . [ label "dump" . store /[0-9]+/ ] .
                    ( sep_spc . [ label "passno" . store /[0-9]+/ ])? )?
                 . eol ]

  let lns = ( empty | comment | record ) *

  let xfm = transform lns (incl "/etc/fstab")

(* Local Variables: *)
(* mode: caml *)
(* End: *)


