(* Parsing /etc/fstab *)

module Fstab =
  autoload xfm

  let sep_tab = Sep.tab
  let sep_spc = Sep.space
  let sep_comma_tab = del /,?[ \t]+/ "\t"
  let comma   = Sep.comma
  let eol     = Util.eol

  let comment = Util.comment
  let empty   = Util.empty

  let file    = /[^# \t\n]+/

  let spec    = /[^,# \n\t][^ \n\t]*/

  (* A vfstype, usually just a short string like "ext3" or "fuse.sshfs", but
     be generous here *)
  let vfslabel = /[^,#= \n\t]+/

  let vfstype_list (l:string) =
      let lns = [ label l . store vfslabel ] in
         Build.opt_list lns comma

  (* A mount option label can't contain comma, comment, equals, or space *)
  let mntoptlabel = /[^,#= \n\t]+/

  let mntopt_list (l:string) =
    let value = [ label "value" . Util.del_str "=" . ( store Rx.neg1 )? ] in
      let lns = [ label l . store mntoptlabel . value? ] in
         Build.opt_list lns comma+

  let record = [ seq "mntent" .
                   Util.indent .
                   [ label "spec" . store spec ] . sep_tab .
                   [ label "file" . store file ] . sep_tab .
                   vfstype_list "vfstype" .
                   (sep_tab . mntopt_list "opt" .
                    (sep_comma_tab . [ label "dump" . store /[0-9]+/ ] .
                     ( sep_spc . [ label "passno" . store /[0-9]+/ ])? )? )?
                 . Util.comment_or_eol ]

  let lns = ( empty | comment | record ) *
  let filter = incl "/etc/fstab"
             . incl "/etc/mtab"

  let xfm = transform lns filter

(* Local Variables: *)
(* mode: caml *)
(* End: *)
