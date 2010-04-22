(* Parses entries in /etc/securetty

   Author: Simon Josi <josi@yokto.net> 
*)
module Securetty =
   autoload xfm

   let word = /[^ \t\n#]+/
   let eol = Util.eol
   let empty = Util.empty
   let comment = Util.comment

   let record = [ seq "securetty" . store word . (comment|eol) ]
   let lns = ( empty | comment | record )*

   let filter = (incl "/etc/securetty")
   let xfm = transform lns filter
