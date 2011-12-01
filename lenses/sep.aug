(*
Module: Sep
   Generic separators to build lenses

Author: Raphael Pinson <raphink@gmail.com>

About: License
  This file is licensed under the LGPLv2+, like the rest of Augeas.
*)


module Sep =

(* Variable: colon *)
let colon = Util.del_str ":"

(* Variable: comma *)
let comma = Util.del_str ","

(* Variable: equal *)
let equal = Util.del_str "="

(* Variable: space
   Deletes a <Rx.space> and default to a single space *)
let space = del Rx.space " "

(* Variable: tab
   Deletes a <Rx.space> and default to a tab *)
let tab   = del Rx.space "\t"

(* Variable: opt_space
   Deletes a <Rx.opt_space> and default to an empty string *)
let opt_space = del Rx.opt_space ""

(* Variable: opt_tab
   Deletes a <Rx.opt_space> and default to a tab *)
let opt_tab   = del Rx.opt_space "\t"
