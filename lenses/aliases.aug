(* Parse mail aliases in /etc/aliases *)
module Aliases =

   let word = /[^, \t\n]+/
   let name = /([^ \t\n#:@]+|"[^"\n]*")/

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
  (* def files = apply lens (include "/etc/aliases" "/system/config/aliases") *)

(* Local Variables: *)
(* mode: caml *)
(* End: *)
