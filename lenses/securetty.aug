(* Parses entries in /etc/securetty

   Author: Simon Josi <josi@yokto.net> 
*)
module Securetty =
   autoload xfm

   let word = /[^ \t\n]+/
   let eol = Util.eol

   let record = [ seq "securetty" . store word . eol ]
   let lns = record*

   let filter = (incl "/etc/securetty")
   let xfm = transform lns filter
