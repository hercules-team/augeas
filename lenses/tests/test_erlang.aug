(*
Module: Test_Erlang
  Provides unit tests and examples for the <Erlang> lens.
*)
module Test_Erlang =

(* Group: comments *)
test Erlang.comment get "% This is a comment\n" =
  { "#comment" = "This is a comment" }

(* Group: simple values *)

let value_bare = Erlang.value Rx.word Erlang.bare

test value_bare get "{foo, bar}" = { "foo" = "bar" }

let value_decimal = Erlang.value Rx.word Erlang.decimal

test value_bare get "{foo, 0.25}" = { "foo" = "0.25" }

let value_quoted = Erlang.value Rx.word Erlang.quoted

test value_quoted get "{foo, '0.25'}" = { "foo" = "0.25" }

let value_glob = Erlang.value Rx.word Erlang.glob

test value_glob get "{foo, <<\".*\">>}" = { "foo" = ".*" }

let value_boolean = Erlang.value Rx.word Erlang.boolean

test value_boolean get "{foo, false}" = { "foo" = "false" }


(* Group: list values *)

let list_bare = Erlang.value_list Rx.word Erlang.bare

test list_bare get "{foo, [bar, baz]}" =
  { "foo"
    { "value" = "bar" }
    { "value" = "baz" } }

(* Group: tuple values *)

let tuple_bare = Erlang.tuple Erlang.bare Erlang.bare

test tuple_bare get "{foo, bar}" =
  { "tuple"
    { "value" = "foo" }
    { "value" = "bar" } }

(* Group: application *)

let list_bare_app = Erlang.application (Rx.word - "kernel") list_bare

test list_bare_app get "{foo, [{bar, [baz, bat]}]}" =
  { "foo"
    { "bar"
      { "value" = "baz" }
      { "value" = "bat" } } }

(* no settings *)
test list_bare_app get "{foo, []}" =
  { "foo" }

(* Group: kernel *)

test Erlang.kernel get "{kernel, [
  {browser_cmd, \"/foo/bar\"},
  {dist_auto_connect, once},
  {error_logger, tty},
  {net_setuptime, 5},
  {start_dist_ac, true}
]}" =
  { "kernel"
    { "browser_cmd" = "/foo/bar" }
    { "dist_auto_connect" = "once" }
    { "error_logger" = "tty" }
    { "net_setuptime" = "5" }
    { "start_dist_ac" = "true" } }

(* Group: config *)

let list_bare_config = Erlang.config list_bare_app

test list_bare_config get "[
  {foo, [{bar, [baz, bat]}]},
  {goo, [{gar, [gaz, gat]}]}
  ].\n" =
  { "foo"
    { "bar"
      { "value" = "baz" }
      { "value" = "bat" } } }
  { "goo"
    { "gar"
      { "value" = "gaz" }
      { "value" = "gat" } } }

(* Test Erlang's kernel app config is parsed *)
test list_bare_config get "[
  {foo, [{bar, [baz, bat]}]},
  {kernel, [{start_timer, true}]}
  ].\n" =
  { "foo"
    { "bar"
      { "value" = "baz" }
      { "value" = "bat" } } }
  { "kernel"
    { "start_timer" = "true" } }
