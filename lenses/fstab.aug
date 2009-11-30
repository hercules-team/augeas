(* Parsing /etc/fstab *)

module Fstab =
  autoload xfm

  let sep_tab = Sep.tab
  let sep_spc = Sep.space
  let comma   = Sep.comma
  let eol     = Util.eol

  let comment = Util.comment
  let empty   = Util.empty

  let word    = Rx.neg1
  let spec    = /[^,# \n\t][^ \n\t]*/

  let comma_sep_list (l:string) =
    let lns = [ label l . store word ] in
       Build.opt_list lns comma

  let record = [ seq "mntent" .
                   [ label "spec" . store spec ] . sep_tab .
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
