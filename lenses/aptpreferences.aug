(*
Module: AptPreferences
  Apt/preferences module for Augeas

Author: Raphael Pinson <raphink@gmail.com>
*)

module AptPreferences =
   autoload xfm

   (************************************************************************
    * Group: Useful primitives and functions
    ************************************************************************)
   (* View: colon *)
   let colon        = del /:[ \t]*/ ": "
   (* View: eol *)
   let eol          = del /[ \t]*\n/ "\n"
   (* View: value_to_eol *)
   let value_to_eol = store /([^ \t\n].*[^ \t\n]|[^ \t\n])/
   (* View: value_to_spc *)
   let value_to_spc = store /[^, \t\n]+/
   (* View: comma *)
   let comma = del /,[ \t]*/ ", "
   (* View: equal *)
   let equal = Util.del_str "="
   (* View: spc *)
   let spc   = Util.del_ws_spc
   (* View: empty *)
   let empty = [ del /[ \t]*\n/ "\n" ]

   (************************************************************************
    * View: simple_entry
    *
    *   Parameters:
    *     kw:string - the pattern to match as key
    ************************************************************************)
   let simple_entry (kw:string) = [ key kw . colon . value_to_eol . eol ]

   (************************************************************************
    * View: key_value
    *
    *   Parameters:
    *     kw:string - the pattern to match as key
    ************************************************************************)
   let key_value (kw:string)    = [ key kw . equal . value_to_spc ]

   (************************************************************************
    * Group: Keywords
    ************************************************************************)
   (* View: pin_keys *)
   let pin_keys = key_value "a"
                | key_value "c"
                | key_value "l"
                | key_value "n"
                | key_value "o"
                | key_value "v"

   (* View: pin_options *)
   let pin_options = store "release" . spc . pin_keys . ( comma . pin_keys )*
   (* View: version_pin *)
   let version_pin = store "version" . [ label "version" . spc . store /[^ \t\n]+/ ]
   (* View: origin_pin *)
   let origin_pin = store "origin" . [ label "origin" . spc . store /[^ \t\n]+/ ]
   (* View: pin *)
   let pin = [ key "Pin" . colon . (pin_options | version_pin | origin_pin) . eol ]

   (* View: entries *)
   let entries = simple_entry "Explanation"
               | simple_entry "Package"
               | simple_entry "Pin-Priority"
               | pin
               | Util.comment

   (* View: record *)
   let record = [ seq "record" . entries+ ]

   (************************************************************************
    * Group: Lens
    ************************************************************************)
   (* View: lns *)
   let lns = (eol* . ( record . eol+ )* . record . eol* ) | eol

   (* View: filter *)
   let filter = incl "/etc/apt/preferences"

   let xfm = transform lns filter
