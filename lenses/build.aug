(*
Module: Build
   Generic functions to build lenses

Author: Raphael Pinson <raphink@gmail.com>

About: License
  This file is licensed under the LGPLv2+, like the rest of Augeas.
*)


module Build =

let eol = Util.eol

(* Generic constructions *)
let brackets (l:lens) (r:lens) (lns:lens) = l . lns . r

(* List constructions *)
let list (lns:lens) (sep:lens) = lns . ( sep . lns )+
let opt_list (lns:lens) (sep:lens) = lns . ( sep . lns )*

(* Labels *)
let xchg (m:regexp) (d:string) (l:string) = del m d . label l

let xchgs (m:string) (l:string) = xchg m m l

(* Keys *)
let key_value_line (kw: regexp) (sep:lens) (sto:lens) =
                                   [ key kw . sep . sto . eol ]

let key_value (kw: regexp) (sep:lens) (sto:lens) =
                                   [ key kw . sep . sto ]
