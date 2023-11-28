(*
Module: Aptsources822
  Augeas module for sources list (Deb822 style) for Apt package manager

Authors:
  James Valleroy <jvalleroy@mailbox.org>
  Sunil Mohan Adapa <sunil@medhas.org>

About: Reference
  1. Deb822(5):
     https://manpages.debian.org/bullseye/dpkg-dev/deb822.5.en.html
  2. sources.list(5):
     https://manpages.debian.org/bullseye/apt/sources.list.5.en.html

About: License
  This file is licensed under the LGPLv2+, like the rest of Augeas.

About: Configuration files
  This lens applies to files in /etc/apt/sources.list.d/ ending with the
  extension .sources. See <filter>.

*)


module Aptsources822 =

autoload xfm

(* Variable: single_value_field_name
   Names of known fields for which only a single value is allowed. *)
let single_value_field_name = /(Enabled|PDiffs|By-Hash|Allow-Insecure|Allow-Weak|Allow-Downgrade-To-Insecure|Trusted|Check-Valid-Until|Valid-Until-Min|Valid-Until-Max|Check-Date|Date-Max-Future|InRelease-Path)/

(* Variable: multi_value_field_name
   Names of known fields for which multiple values are allowed and names of
   unknown fields. Unknown fields are assumed to contain multiple values as
   that is the safest assumption.

   According to deb822(5) man page, "The field name is composed of US-ASCII
   characters excluding control characters, space, and colon (i.e., characters
   in the ranges U+0021 ‘!’ through U+0039 ‘9’, and U+003B ‘;’ through U+007E
   ‘~’, inclusive). Field names must not begin with the comment character
   (U+0023 ‘#’), nor with the hyphen character (U+002D ‘-’)." *)
let multi_value_field_name = /[!"$-,.-9;-~][!-9;-~]*/ - single_value_field_name

(* Variable: field_value
   Value that a field can contain. Deb822 styles sources list files defines some
   fields to have multiple values separated by space, tab or a newline. *)
let field_value = /[!-Z\\^-~][!-Z\\^-~]*/

(* Variable: empty_line
   Lens for an empty line separating two stanzas. It can't be a comment. Only
   tabs and spaces are allowed. *)
let empty_line = Util.empty_generic /[ \t]*/

(* Variable: name_value_separator
   Lens for separating a name and value. Field name is followed by a ':' and
   then optionally space. The file format also allow for a value to be on a
   new line when the new line starts with space or a tab. *)
let name_value_separator = Sep.colon . del (Rx.opt_space . /(\n[ \t])?/) " "

(* Variable: field_value_with_newline
   Lens for value that followed by a new line and a space. This indicates that
   another value follows it. *)
let field_value_with_newline = [seq "item" . store (field_value . /\n/) .
   del /[\t ]/ " "]

(* Variable: field_value_with_separator
   Lens for value that followed by a space or a tab. This indicates that another
   value follows it. *)
let field_value_with_separator = [seq "item" . store field_value . del Rx.space " "]

(* Variable: field_value_with_eol
   Lens for value that followed by an end-of-line. This indicates that this is
   the last value for this field. *)
let field_value_with_eol = [seq "item" . store field_value . Util.eol]

(* Variable: single_value_field
   Lens for a field (field name, separator and field value) with only a single
   value. *)
let single_value_field = [ key single_value_field_name . name_value_separator .
  store field_value . Util.eol ]

(* Variable: multi_value_field
   Lens for a field (field name, separator and field value) with multiple values
   *)
let multi_value_field = [ key multi_value_field_name . name_value_separator .
  counter "item" . (field_value_with_newline | field_value_with_separator)* .
  field_value_with_eol ]

(* Variable: stanza
   Lens for a stanza that describes one or more sources for apt. *)
let stanza = [ seq "source" . (single_value_field | multi_value_field |
  Util.comment_noindent)+ ]

(* Variable: lns
   Lens for parsing the entire apt sources file in Deb822 format. *)
let lns = stanza . (empty_line . stanza)*

(* Variable: filter
   All files in the sources.list.d directory are files describing sources.
   However, only those ending with .sources extension are in Deb822 format. *)
let filter = incl "/etc/apt/sources.list.d/*.sources"

let xfm = transform lns filter
