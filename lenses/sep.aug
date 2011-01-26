(*
Module: Sep
   Generic separators to build lenses

Author: Raphael Pinson <raphink@gmail.com>

About: License
  This file is licensed under the LGPLv2+, like the rest of Augeas.
*)


module Sep =

let colon = Util.del_str ":"
let comma = Util.del_str ","
let space = del Rx.space " "
let tab   = del Rx.space "\t"
let opt_space = del Rx.opt_space ""
let opt_tab   = del Rx.opt_space "\t"

