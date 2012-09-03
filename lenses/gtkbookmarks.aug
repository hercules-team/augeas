(*
Module: GtkBookmarks
  Parses $HOME/.gtk-bookmarks

Author: Raphael Pinson <raphael.pinson@camptocamp.com>

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to $HOME/.gtk-bookmarks. See <filter>.

About: Examples
   The <Test_GtkBookmarks> file contains various examples and tests.
*)
module GtkBookmarks =

autoload xfm

(* View: entry *)
let entry = [ seq "entry" . store Rx.no_spaces
            . (Sep.space . [ label "label" . store Rx.space_in ])?
            . Util.eol ]

(* View: lns *)
let lns = entry*

(* View: xfm *)
let xfm = transform lns (incl (Sys.getenv("HOME") . "/.gtk-bookmarks"))
