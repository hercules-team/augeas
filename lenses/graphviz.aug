(*
Module: Graphviz
  Parses graphviz files.

Author: Raphael Pinson <raphink@gmail.com>

About: Reference

About: License
   This file is licenced under the LGPLv2+, like the rest of Augeas.

About: Lens Usage
   To be documented

About: Configuration files
   This lens applies to graphviz files.

About: Examples
   The <Test_Graphviz> file contains various examples and tests.
*)

module Graphviz =

(* Variable: name_re *)
let name_re = /[A-Za-z0-9_.]+/

(* View: graph_name *)
let graph_name = 
   store ( ("\"" . /[^"]*/ . "\"") | name_re )

(* View: name *)
let node_name =
   let reserved = "node" | "graph"
   in  store ( ("\"" . /[^"]*/ . "\"") | (name_re - reserved) )

(* View: eol
     Special eol using semicolons and/or newlines *)
let eol = del /[ \t]*((;([ \t\n]*\n)?)|\n([ \t]*\n)?)/ ";\n"

(* View: block
     A block construct, using Build.block_generic
     in order to pass our special definition of <eol> *)
let block (kw:regexp) (entry:lens) =
   [ key kw
   . Sep.space . graph_name
   . Build.block_generic  (* Use our definition of eol *)
        (Util.indent . entry . eol)
        (entry . eol) (Util.indent . entry) entry
        Util.comment Util.comment_noindent
        Build.block_ldelim_re /;?[ \t\n]*\}/ (* Allow semicol *)
        Build.block_ldelim_default Build.block_rdelim_default ]

(* View: variable_generic
     A generic way of parsing variables
     with or without double quotes *)
let variable_generic (kw:regexp) =
   Build.key_value kw Sep.space_equal node_name

(* View: variable *)
let variable =
   let reserved = "subgraph" | "node"
   in variable_generic (Rx.word - reserved)

(* View: options
     A list of options for <nodes> or <links> *)
let options (kw:regexp) = 
   let option = variable_generic kw
   in let comma = del /,[ \t\n]*/ ","
   in Sep.opt_space . del /\[[ \t]*/ "["
                . Build.opt_list option comma
                . del /[ \t]*\]/ "]"

(* View: link *)
let link =
      let link_node = [ label "node" . node_name ]
   in let link_type = [ label "link_type" . Sep.opt_space
                      . store /\<?\-\-?\>?/ . Sep.opt_space ]
   in [ label "link"
      . Build.list link_node link_type
      . (options (Rx.word - "link_type"))? ]

(* View: node *)
let node = [ label "node" . node_name . (options Rx.word)? ]

(* View: nodelist
     Several <nodes> can be declared at once in a nodelist.
     This cannot be declared as simply a list of <nodes>,
     since it would be ambiguous in the put direction. *)
let nodelist = [ label "nodelist" . Build.opt_list node Sep.space ]

(* View: default
     Default options for <nodes> or <graphs> *)
let default = [ label "@default" . store ("node" | "graph")
              . options Rx.word ]

(* View: entry
     A general entry *)
let entry = (link | variable | nodelist | default)

(* View: subgraph_nonrec
     Non recursive lens, for tests *)
let subgraph_nonrec = block "subgraph" entry

(* View: subgraph
     Recursive mode doesn't work in the put direction
     so this is resctricted to two levels for now.
     TODO: See why recursive mode won't work. *)
let subgraph = block "subgraph" (subgraph_nonrec | entry)

(* View: graph
     The top-level graph
     Use <subgraph_nonrec> for debug *)
let graph = block /(di)?graph/ (subgraph | entry) . Util.eol

(* View: lns
     The graphviz lens *)
let lns = (Util.comment | Util.empty)* . graph . (Util.comment | Util.empty)*
