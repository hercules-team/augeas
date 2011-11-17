(*
Module: Build
   Generic functions to build lenses

Author: Raphael Pinson <raphink@gmail.com>

About: License
  This file is licensed under the LGPLv2+, like the rest of Augeas.

About: Reference
  This file provides generic functions to build Augeas lenses
*)


module Build =

let eol = Util.eol

(************************************************************************
 * Group:               GENERIC CONSTRUCTIONS
 ************************************************************************)

(************************************************************************
 * View: brackets
 *   Put a lens inside brackets
 *
 *   Parameters:
 *     l:lens   - the left bracket lens
 *     r: lens  - the right bracket lens
 *     lns:lens - the lens to put inside brackets
 ************************************************************************)
let brackets (l:lens) (r:lens) (lns:lens) = l . lns . r


(************************************************************************
 * Group:             LIST CONSTRUCTIONS
 ************************************************************************)

(************************************************************************
 * View: list
 *   Build a list of identical lenses separated with a given separator
 *   (at least 2 elements)
 *
 *   Parameters:
 *     lns:lens - the lens to repeat in the list
 *     sep:lens - the separator lens, which can be taken from the <Sep> module
 ************************************************************************)
let list (lns:lens) (sep:lens) = lns . ( sep . lns )+


(************************************************************************
 * View: opt_list
 *   Same as <list>, but there might be only one element in the list
 *
 *   Parameters:
 *     lns:lens - the lens to repeat in the list
 *     sep:lens - the separator lens, which can be taken from the <Sep> module
 ************************************************************************)
let opt_list (lns:lens) (sep:lens) = lns . ( sep . lns )*


(************************************************************************
 * Group:                   LABEL OPERATIONS
 ************************************************************************)

(************************************************************************
 * View: xchg
 *   Replace a pattern with a different label in the tree,
 *   thus emulating a key but allowing to replace the keyword
 *   with a different value than matched
 *
 *   Parameters:
 *     m:regexp - the pattern to match
 *     d:string - the default value when a node in created
 *     l:string - the label to apply for such nodes
 ************************************************************************)
let xchg (m:regexp) (d:string) (l:string) = del m d . label l

(************************************************************************
 * View: xchgs
 *   Same as <xchg>, but the pattern is the default string
 *
 *   Parameters:
 *     m:string - the string to replace, also used as default
 *     l:string - the label to apply for such nodes
 ************************************************************************)
let xchgs (m:string) (l:string) = xchg m m l


(************************************************************************
 * Group:                   SUBNODE CONSTRUCTIONS
 ************************************************************************)

(************************************************************************
 * View: key_value_line
 *   A subnode with a keyword, a separator and a storing lens,
 *   and an end of line
 *
 *   Parameters:
 *     kw:regexp - the pattern to match as key
 *     sep:lens  - the separator lens, which can be taken from the <Sep> module
 *     sto:lens  - the storing lens
 ************************************************************************)
let key_value_line (kw:regexp) (sep:lens) (sto:lens) =
                                   [ key kw . sep . sto . eol ]

(************************************************************************
 * View: key_value_line_comment
 *   Same as <key_value_line>, but allows to have a comment in the end of a line
 *   and an end of line
 *
 *   Parameters:
 *     kw:regexp    - the pattern to match as key
 *     sep:lens     - the separator lens, which can be taken from the <Sep> module
 *     sto:lens     - the storing lens
 *     comment:lens - the comment lens, which can be taken from <Util>
 ************************************************************************)
let key_value_line_comment (kw:regexp) (sep:lens) (sto:lens) (comment:lens) =
                                   [ key kw . sep . sto . (eol|comment) ]

(************************************************************************
 * View: key_value
 *   Same as <key_value_line>, but does not end with an end of line
 *
 *   Parameters:
 *     kw:regexp - the pattern to match as key
 *     sep:lens  - the separator lens, which can be taken from the <Sep> module
 *     sto:lens  - the storing lens
 ************************************************************************)
let key_value (kw: regexp) (sep:lens) (sto:lens) =
                                   [ key kw . sep . sto ]

(************************************************************************
 * View: key_ws_value
 *
 *   Store a key/value pair where key and value are separated by whitespace
 *   and the value goes to the end of the line. Leading and trailing
 *   whitespace is stripped from the value. The end of line is consumed by
 *   this lens
 *
 *   Parameters:
 *     kw:regexp - the pattern to match as key
 ************************************************************************)
let key_ws_value (kw:regexp) =
  key_value_line kw Util.del_ws_spc (store Rx.space_in)

(************************************************************************
 * View: flag
 *   A simple flag subnode, consisting of a single key
 *
 *   Parameters:
 *     kw:regexp - the pattern to match as key
 ************************************************************************)
let flag (kw:regexp) = [ key kw ]

(************************************************************************
 * View: flag_line
 *   A simple flag line, consisting of a single key
 *
 *   Parameters:
 *     kw:regexp - the pattern to match as key
 ************************************************************************)
let flag_line (kw:regexp) = [ key kw . eol ]

