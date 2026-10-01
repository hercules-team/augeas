(* Parsing /etc/fstab *)

module Fstab =
  autoload xfm

  let sep_tab = Sep.tab
  let sep_spc = Sep.space

  let comma   = Sep.comma

  let file    = /[^# \t\n]+/
  let spec    = /[^,# \n\t][^ \n\t]*/

  (* vfstype - a list of allowed filesystem type, typically just a
     short string like ext4 or iso9660, but it may be a comma-separated
     list, and contain entries like fuse.sshfs
  *)
  let vfstype_list =
      let lns = [ label "vfstype" . store /[^,#= \n\t]+/ ] in
         Build.opt_list lns comma

  (* An option label can't contain comma, comment, equals, or space *)
  let opt_1_label = /[^,#= \n\t]+/
  let opt_n_label = /[^,#= \n\t]*/

  let value       = [ label "value" . del "=" "=" . ( store /[^ \t\n=,#]*/ ) ]
  let opt_1       = [ label "opt" .   store opt_1_label . value?    ]
  let opt_n       = [ label "opt" . ( store opt_n_label . value? )? ]

  let mntopt_list = ( opt_1
                    | opt_n ? . ( comma . opt_n ? )+
                    )

  let record = [ seq "mntent" .
                   Util.indent .
                   [ label "spec" . store spec ] . sep_tab .
                   [ label "file" . store file ] . sep_tab .
                   vfstype_list . ( sep_tab .
                   mntopt_list  .   ( sep_tab .
                   [ label "dump"   . store /[0-9]+/ ] . ( sep_spc .
                   [ label "passno" . store /[0-9]+/ ]   )?
                                    )?
                                  )?
                 . Util.comment_or_eol
               ]
(* Local Variables: *)
(* mode: caml *)
(* End: *)
