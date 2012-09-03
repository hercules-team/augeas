(*
Module: Test_GtkBookmarks
  Provides unit tests and examples for the <GtkBookmarks> lens.
*)

module Test_GtkBookmarks =

(* Test: GtkBookmarks.lns
     Test without label *)
test GtkBookmarks.lns get "ftp://user@myftp.com/somedir\n" =
 { "bookmark" = "ftp://user@myftp.com/somedir" }

(* Test: GtkBookmarks.lns
     Test with label *)
test GtkBookmarks.lns get "file:///home/rpinson/Ubuntu%20One Ubuntu One\n" =
 { "bookmark" = "file:///home/rpinson/Ubuntu%20One"
   { "label" = "Ubuntu One" } }

(* Test: GtkBookmarks.lns
     Empty lines are allowed, not comments *)
test GtkBookmarks.lns get "ftp://user@myftp.com/somedir\n\nfile:///home/rpinson/Ubuntu%20One Ubuntu One\n" =
 { "bookmark" = "ftp://user@myftp.com/somedir" }
 { }
 { "bookmark" = "file:///home/rpinson/Ubuntu%20One"
   { "label" = "Ubuntu One" } }
