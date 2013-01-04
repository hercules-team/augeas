(*
Module: Aptsources
  Parsing /etc/apt/sources.list
*)

module Aptsources =
  autoload xfm

(************************************************************************
 * Group: Utility variables/functions
 ************************************************************************)
  (* View:  sep_ws *)
  let sep_ws = del /[ \t]+/ " "

  (* View: eol *)
  let eol = Util.del_str "\n"

  (* View: comment *)
  let comment = Util.comment
  (* View: empty *)
  let empty = Util.empty

  (* View: word *)
  let word = /[^# \n\t]+/

(************************************************************************
 * Group: Keywords
 ************************************************************************)
  (* View: record *)
  let record = [ Util.indent . seq "source" . [ label "type" . store word ] . sep_ws .
                                [ label "uri"  . store word ] . sep_ws .
                                [ label "distribution" . store word ]  .
                                [ label "component" . sep_ws . store word ]* .
                                del /[ \t]*(#.*)?/ ""
                 . eol ]

(************************************************************************
 * Group: Lens
 ************************************************************************)
  (* View: lns *)
  let lns = ( comment | empty | record ) *

  (* View: filter *)
  let filter = (incl "/etc/apt/sources.list")
      . (incl "/etc/apt/sources.list.d/*")
      . Util.stdexcl

  let xfm = transform lns filter

(* Local Variables: *)
(* mode: caml *)
(* End: *)
