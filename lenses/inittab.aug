(* Parsing /etc/inittab *)
module Inittab =
   autoload xfm

   let sep = Util.del_str ":"
   let eol = Util.del_str "\n"

   let value = /[^#:\n]*/

   let comment = [ del /[ \t]*(#.*)?\n/ "# \n" ]

   let field (name:string) = [ label name . store value ]
   let record = [ seq "record" . 
                    field("id") . sep .
                    field("runlevels") . sep .
                    field("action") . sep .
                    field("process") . 
                    eol
                ]

   let lns = ( comment | record ) *

   let xfm = transform lns (incl "/etc/inittab")


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
