(* Parse mail aliases in /etc/aliases *)
module Aliases =
   autoload xfm

   let word = /[^, \t\n]+/
   let name = /([^ \t\n#:@]+|"[^"\n]*")/ (* " make emacs calm down *)

   let colon = del /:[ \t]+/ ":\t"
   let eol   = Util.eol

   let comment = Util.comment
   let empty   = Util.empty

   let comma = del /,[ \t]*(\n[ \t]+)?/ ", "
   let value_list = Build.opt_list ([ label "value" . store word]) comma
   let alias = [ seq "alias" .
                    [ label "name" . store name ] . colon .
                    value_list
                ] . eol

  let lns = (comment | empty | alias)*

  let xfm = transform lns (incl "/etc/aliases")

(* Local Variables: *)
(* mode: caml *)
(* End: *)
