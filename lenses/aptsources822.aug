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
let single_value_field_name = /(Enabled|PDiffs|By-Hash|Allow-Insecure|Allow-Weak|Allow-Downgrade-To-Insecure|Trusted|Check-Valid-Until|Valid-Until-Min|Valid-Until-Max|Check-Date|Date-Max-Future|InRelease-Path|Snapshot)/

(* Variable: multi_value_field_name
   Names of known fields for which multiple values are allowed
   and are assumed to contain multiple values
   These fields also allow the form
     FieldName-Add: extra_value
     FieldName-Remove: delete_this_value
   This is an explicit list of allowed field names *)
let multi_value_field_name = /(URIs|Types|Components|Suites|Architectures|Include|Exclude|Languages|Targets|Signed-By)(-Add|-Remove)?/

(* Variable: field_value
   Value that a field can contain. Deb822 styles sources list files defines these
   fields to have multiple values separated by space, tab or a newline. *)
let field_value = /[!-~]+/

(* Variable: text_value
   Value for the embedded public-key format for Signed-By
   Includes all text on the line except leading-spaces and trailing-spaces *)
let text_value = /[!-~](.*[!-~])?/

(* Variable: empty_line
   Lens for an empty line separating two stanzas. It can't be a comment. Only
   tabs and spaces are allowed. *)
let empty_line = Util.doseol

(* Variable: name_value_separator
   Lens for separating a name and value. Field name is followed by a ':' and
   then optionally space. The file format also allow for a value to be on a
   new line when the new line starts with space or a tab. *)
let name_value_separator = Sep.colon . del Rx.opt_space " "

(* Variable: single_value_field
   Lens for a field (field name, separator and field value) with only a single
   value. *)
let single_value_field = [ key single_value_field_name . name_value_separator .
  store field_value . Util.doseol ]

(* Variable: multi_value_field
   Lens for a field (field name, separator and field value) with multiple values
   or can run over multiple lines, ie. SignedBy
*)
let multi_value_field = [ key multi_value_field_name . name_value_separator .
    (
      ( counter "item" . [ seq "item". store field_value . del /[ \t]+/ " " ]* . [ seq "item". store field_value ] . Util.doseol  ) |
      ( [ label "text" . del "\n" "\n" .
          counter "item" . [ del /^[ ]/ " " . seq "item" . store text_value . Util.doseol ]+
        ]
      )
    )
  ]

(* Variable: stanza
   Lens for a stanza that describes one or more sources for apt. *)
(* An empty_line must seperate stanzas.
   A stanza may be completely empty, but must end on a newline
   (or end at the end-of-file)
   This results in the counter "source" being incremented with
   each empty line *)
let stanza = [ seq "source" . (single_value_field | multi_value_field |
  Util.comment_noindent)* ]

(* Variable: lns
   Lens for parsing the entire apt sources file in Deb822 format. *)
let lns = ( stanza . empty_line )* . stanza

(* Variable: filter
   All files in the sources.list.d directory are files describing sources.
   However, only those ending with .sources extension are in Deb822 format. *)
let filter = incl "/etc/apt/sources.list.d/*.sources"

let xfm = transform lns filter
