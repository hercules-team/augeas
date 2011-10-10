(* Parse mail aliases in /etc/aliases *)
module Aliases =
   autoload xfm

   (************************************************************************
    * Group:                 USEFUL PRIMITIVES
    *************************************************************************)

   (* Group: basic tokens *)

   (* Variable: word *)
   let word = /[^\|", \t\n]+/
   (* Variable: name *)
   let name = /([^ \t\n#:|@]+|"[^"|\n]*")/ (* " make emacs calm down *)

   (* Variable: command
    * a command can contain spaces, if enclosed in double quotes, the case
    * without spaces is taken care with <word>
    *)
   let command = /\|([^", \t\n]+|"[^"\n]+")/

   (* Group: Comments and empty lines *)

   (* View: eol *)
   let eol   = Util.eol
   (* View: comment *)
   let comment = Util.comment
   (* View: empty *)
   let empty   = Util.empty

   (* Group: separators *)
   (* View: colon
    * Separation between the alias and it's destinations
    *)
   let colon = del /:[ \t]+/ ":\t"
   (* View: comma
    * Separation between multiple destinations
    *)
   let comma = del /[ \t]*,[ \t]*(\n[ \t]+)?/ ", "

   (* Group: alias *)

   (* View: destination
    * Can be either a word (no spaces included) or a command with spaces
    *)
   let destination = ( word | command )

   (* View: value_list
    * List of destinations
    *)
   let value_list = Build.opt_list ([ label "value" . store destination]) comma

   (* View: alias
    * a name with one or more destinations
    *)
   let alias = [ seq "alias" .
                    [ label "name" . store name ] . colon .
                    value_list
                ] . eol

  let lns = (comment | empty | alias)*

  let xfm = transform lns (incl "/etc/aliases")

(* Local Variables: *)
(* mode: caml *)
(* End: *)
