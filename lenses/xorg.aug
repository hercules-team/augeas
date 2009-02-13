(*
Module: Xorg
 Parses /etc/X11/xorg.conf

Author: Raphael Pinson <raphink@gmail.com>

About: Reference
 This lens tries to keep as close as possible to `man xorg.conf` where
 possible.

The definitions from `man xorg.conf` are put as commentaries for reference
throughout the file. More information can be found in the manual.

About: License
  This file is licensed under the GPL.

About: Lens Usage
  Sample usage of this lens in augtool

    * Get the identifier of the devices with a "Clone" option:
      > match "/files/etc/X11/xorg.conf/Device[Option = 'Clone']/Identifier"

About: Quotes in values
  In this file, values may or may not be quoted (with double quotes) unless
they contain spaces or tabulations. For this reason, the quotes are
included in the parsed values when present. New values with no spaces will
not have quotes by default, unless you type the quotes yourself around
them. New values with spaces will not be accepted unless they are
explicitely surrounded by double qutoes.

About: Configuration files
  This lens applies to /etc/X11/xorg.conf. See <filter>.
*)

module Xorg =
  autoload xfm

(************************************************************************
 * Group:                 USEFUL PRIMITIVES
 *************************************************************************)

(* Group: Generic primitives *)

(* Variable: eol *)
let eol     = Util.eol

(* Variable: indent *)
let indent  = Util.indent

(* Variable: comment *)
let comment = Util.comment

(* Variable: empty *)
let empty   = Util.empty


(* Group: Separators *)

(* Variable: sep_spc *)
let sep_spc = Util.del_ws_spc

(* Variable: sep_dquote *)
let sep_dquote  = Util.del_str "\""


(* Group: Fields and values *)

(* Variable: entry_re *)
let entry_re    = /[^# \t\n\/]+/ - "Option"


(* Variable: word_space
 *  TODO: refine possible values by Section
 *  words with spaces require quotes
 *)
let word_space  = /"[^"\n]+"/

(*
 * Variable: word
 *   Words without spaces may have quotes or not
 *   the quotes are then part of the value
 *)
let word        = /[^" \t\n]+/

(* Variable: quoted_word *)
let quoted_word = /"[^" \t\n]+"/   (* " relax Emacs *)

(* Variable: word_all *)
let word_all = word_space | word | quoted_word



(************************************************************************
 * Group:                          ENTRIES AND OPTIONS
 *************************************************************************)


(* View: entry *)
let entry  = [ indent . key entry_re
                . sep_spc . store word_all . eol ]

(* View: option_value *)
let option_value =
                [ label "value" . store word_all ]

(* View: option *)
let option = [ indent . key "Option" . sep_spc
                . store word_all
                . (sep_spc . option_value)* . eol ]

(* View: section_entry *)
let section_entry = empty | comment | entry | option


(************************************************************************
 * Group:                       SECTIONS
 *************************************************************************)


(************************************************************************
 * Variable: section_re
 *   Known values for Section names
 *
 *   Definition:
 *     >   The section names are:
 *     >
 *     >   Files          File pathnames
 *     >   ServerFlags    Server flags
 *     >   Module         Dynamic module loading
 *     >   InputDevice    Input device description
 *     >   Device         Graphics device description
 *     >   VideoAdaptor   Xv video adaptor description
 *     >   Monitor        Monitor description
 *     >   Modes          Video modes descriptions
 *     >   Screen         Screen configuration
 *     >   ServerLayout   Overall layout
 *     >   DRI            DRI-specific configuration
 *     >   Vendor         Vendor-specific configuration
 *************************************************************************)
let section_re = /(Files|ServerFlags|Module|InputDevice|Device|VideoAdaptor
                        |Monitor|Modes|Screen|ServerLayout|DRI|Vendor)/


(************************************************************************
 * Variable: secton_re_obsolete
 *   The  following obsolete section names are still recognised for
 *   compatibility purposes.  In new config files, the InputDevice
 *   section should be used instead.
 *
 *   Definition:
 *     >  Keyboard       Keyboard configuration
 *     >  Pointer        Pointer/mouse configuration
 *************************************************************************)
let section_re_obsolete = /(Keyboard|Pointer)/

(************************************************************************
 * View: section
 *   A section in xorg.conf
 *
 *   Definition:
 *     > Section  "SectionName"
 *     >    SectionEntry
 *     >    ...
 *     > EndSection
 *************************************************************************)
let section = [ indent . del "Section" "Section"
                       . sep_spc . sep_dquote
                       . key (section_re|section_re_obsolete) . sep_dquote
                       . eol
                .  section_entry*
                . indent . del "EndSection" "EndSection" . eol ]

(*
 * View: lns
 *   The xorg.conf lens
 *)
let lns = ( empty | comment | section )*


(* Variable: filter *)
let filter = (incl "/etc/X11/xorg.conf")

let xfm = transform lns filter
