(* Parse mail aliases in /etc/aliases *)
module Aliases =
   autoload xfm

   let word = /[^, \t\n]+/
   let name = /([^ \t\n#:@]+|"[^"\n]*")/ (* " make emacs calm down *)

   let colon = del /:[ \t]+/ ":\t"
   let eol = del /[ \t]*\n/ "\n"

   let comment = [ del /^[ \t]*(#.*)?\n/ "#\n" ]

   let comma = del /,[ \t]+(\n[ \t]+)?/ ", "
   let alias =  [ seq "alias" .
                    [ label "name" . store name ] . colon .
                    counter "values" .
                    [ label "values" .
                        [ seq "values" . store word ] .
                        ([comma . seq "values" . store word])*
                    ]
                ] . eol

  let lns = (comment | alias)*

  let xfm = transform lns (incl "/etc/aliases")

(* Local Variables: *)
(* mode: caml *)
(* End: *)
