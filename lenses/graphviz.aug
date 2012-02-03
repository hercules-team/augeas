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

(* View: name *)
let name =
   let reserved = "node" | "graph"
   in  store ( ("\"" . /[^"]*/ . "\"")
               | (/[A-Za-z0-9_.]+/ - reserved) )

(* View: eol
     Special eol using semicolons and/or newlines *)
let eol = del /[ \t]*((;\n?)|\n)/ ";\n"

(* View: block
     A block construct, using Build.block_generic
     in order to pass our special definition of <eol> *)
let block (kw:regexp) (entry:lens) =
   [ key kw
   . Sep.space . name
   . Build.block_generic  (* Use our definition of eol *)
        (Util.indent . entry . eol)
        (entry . eol) (Util.indent . entry) entry
        Util.comment Util.comment_noindent
        Build.block_ldelim_re /;?[ \t\n]*\}/ (* Allow semicol *)
        Build.block_ldelim_default Build.block_rdelim_default ]

(* View: variable_generic
     A generic way of parsing variables
     with or without double quotes *)
let variable_generic (reserved:regexp) =
   Build.key_value (Rx.word - reserved) Sep.space_equal name

(* View: variable *)
let variable =
   let reserved = "subgraph" | "node"
   in variable_generic reserved

(* View: options
     A list of options for <nodes> or <links> *)
let options = 
      let reserved = "node" | "link_type"
   in let option = variable_generic reserved
   in let comma = del /,[ \t\n]*/ ","
   in Sep.space . del /\[[ \t]*/ "["
                . Build.opt_list option comma
                . del /[ \t]*\]/ "]"

(* View: link *)
let link =
      let link_node = [ label "node" . name ]
   in let link_type = [ label "link_type" . Sep.opt_space
                      . store /\<?\-\-?\>?/ . Sep.opt_space ]
   in [ label "link"
      . Build.list link_node link_type
      . options? ]

(* View: node *)
let node = [ label "node" . name . options? ]

(* View: nodelist
     Several <nodes> can be declared at once in a nodelist.
     This cannot be declared as simply a list of <nodes>,
     since it would be ambiguous in the put direction. *)
let nodelist = [ label "nodelist" . Build.list node Sep.space ]

(* View: default_node
     General settings for <nodes> *)
let default_node = [ Build.xchgs "node" "@node" . options ]

(* View: default_graph
     General settings for <graphs> *)
let default_graph = [ Build.xchgs "graph" "@graph" . options ]

(* View: entry
     A general entry *)
let entry = (link | variable | node | nodelist
           | default_node | default_graph)

(* View: subgraph
     Recursive *)
let rec subgraph = block "subgraph" (subgraph | entry)

(* View: graph
     The top-level graph *)
let graph = block /(di)?graph/ (subgraph | entry) . Util.eol

(* View: lns
     The graphviz lens *)
let lns = (Util.comment | Util.empty)* . graph


