(* Apt/preferences module for Augeas          *)
(* Author: Raphael Pinson <raphink@gmail.com> *)

module AptPreferences =
   autoload xfm

   (* Define useful primitives *)
   let colon        = del /:[ \t]*/ ": "
   let eol          = del /[ \t]*\n/ "\n"
   let value_to_eol = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/
   let value_to_spc = store /[^, \t\n]+/
   let comma = del /,[ \t]*/ ", "
   let equal = Util.del_str "="
   let spc   = Util.del_ws_spc

   (* Define empty *)
   let empty = [ del /[ \t]*\n/ "" ]

   (* Define record *)

   let simple_entry (kw:string) = [ key kw . colon . value_to_eol . eol ]

   let key_value (kw:string)    = [ key kw . equal . value_to_spc ]
   let pin_keys = key_value "a"
                | key_value "c"
                | key_value "l"
                | key_value "o"
                | key_value "v"

   let pin = [ key "Pin" . colon . value_to_spc . spc . pin_keys . ( comma . pin_keys )*. eol ]

   let entries = simple_entry "Explanation"
               | simple_entry "Package"
               | simple_entry "Pin-Priority"
               | pin

   let record = [ seq "record" . entries+ ]

   (* Define lens *)
   let lns = empty* . ( record . empty )* . record?

   let filter = incl "/etc/apt/preferences"
              . Util.stdexcl

   let xfm = transform lns filter

