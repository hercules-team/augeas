(* Intefraces module for Augeas
 Author: Free Ekanayaka <free@64studio.com>

 Reference: man interfaces

*)

module Interfaces =
   autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol        = Util.eol

(* Define separators *)

(* a line can be extended across multiple lines by making the last  *)
(*  character a backslash, unless the line is a comment (see below) *)
let sep_spc    =  del /([ \t]+|[ \t]*\\\\\n[ \t]*)/ " "

(* Define fields *)
let sto_to_eol = store /([^\\# \t\n].*[^\\ \t\n]|[^\\ \t\n])/
let sto_to_spc = store /[^\\# \t\n]+/

(* Define comments and empty lines *)

(* note that the comment definition from Util does not support *)
(* splitting lines with a backlash                             *)
let comment    = Util.comment

let empty      = Util.empty

(* Define tree stanza_ids *)
let stanza_id    (t:string) = key t . sep_spc . sto_to_spc
let stanza_param (l:string) = [ sep_spc . label l . sto_to_spc ]

(* Define reseverved words *)
let stanza_word = /(iface|auto|allow-[a-z-]+|mapping)/

(* Define additional lines for mluti-line stanzas *)
let stanza_option = [  del /[ \t]*/ "   " 
                     . key  ( /[a-z-]+/ - stanza_word )
                     . sep_spc
                     . sto_to_eol
                     . eol ]

(************************************************************************
 *                              AUTO
 *************************************************************************)

let array (r:regexp) (t:string) =  del r t . label t . counter t
   . [ sep_spc . seq t . sto_to_spc ]+
let auto = [ array /(allow-)?auto/ "auto" . (comment|eol) ]

(************************************************************************
 *                              HOTPLUG
 *************************************************************************)

let hotplug = [ stanza_id "allow-hotplug" . (comment|eol) ]

(************************************************************************
 *                              MAPPING
 *************************************************************************)

let mapping = [ stanza_id "mapping"
               . ( eol | ((eol . empty*)? . comment+) )
               . empty*
               . stanza_option
               . (stanza_option|comment|empty)* ]

(************************************************************************
 *                              IFACE
 *************************************************************************)

let iface   = [ stanza_id    "iface"
              . stanza_param "family"
              . stanza_param "method"
              . ( eol | ((eol . empty*)? . comment+) )
              . empty*
              . ( stanza_option . (stanza_option|comment|empty)* )? ]

(************************************************************************
 *                              STANZAS
 *************************************************************************)

(* The auto and hotplug stanzas always consist of one line only, while
   iface and mapping can spand along more lines. Comment nodes are
   inserted in the tree as direct children of the root node only when they
   come after an auto or hotplug stanza, otherwise they are considered part
   of a iface or mapping block *)

let stanza_single = (auto|hotplug) . (comment|empty)*
let stanza_multi  = iface|mapping

(************************************************************************
 *                              LENS & FILTER
 *************************************************************************)

   let lns = (comment|empty)* . (stanza_multi | stanza_single)*

   let filter = incl "/etc/network/interfaces"
              . Util.stdexcl

   let xfm = transform lns filter

