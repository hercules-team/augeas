(*
Module: Test_Graphviz
  Provides unit tests and examples for the <Graphviz> lens.
*)

module Test_Graphviz =

(* Variable: hello
    From http://www.graphviz.org/content/hello *)
let hello = "# Hello World!

digraph G {Hello->World}\n"

(* Test: Graphviz.lns
    Hello World *)
test Graphviz.lns get hello =
  { "#comment" = "Hello World!" }
  { }
  { "digraph" = "G"
     { "link"
        { "node" = "Hello" }
        { "link_type" = "->" }
        { "node" = "World" } } }

(* Variable: entries
     Simple entries *)
let entries = "graph A {
style=filled;
color=lightgrey;
node [style=filled,color=white];
label = \"process #1\";\n}\n"

(* Test: Graphviz.lns *)
test Graphviz.lns get entries =
  { "graph" = "A"
    { "style" = "filled" }
    { "color" = "lightgrey" }
    { "@node"
      { "style" = "filled" }
      { "color" = "white" } }
    { "label" = "\"process #1\"" } }

(* Variable: simple_cluster *)
let simple_cluster = "digraph G {
	subgraph cluster_0 {
		a0 -> a1 -> a2 -> a3;
	}
}\n"

(* Test: Graphviz.lns
    Simple cluster *)
test Graphviz.lns get simple_cluster =
  { "digraph" = "G"
    { "subgraph" = "cluster_0"
      { "link"
        { "node" = "a0" }
        { "link_type" = "->" }
        { "node" = "a1" }
        { "link_type" = "->" }
        { "node" = "a2" }
        { "link_type" = "->" }
        { "node" = "a3" } } } }

(* Variable: cluster
      From http://www.graphviz.org/content/cluster *)
let cluster = "digraph G {

	subgraph cluster_0 {
		style=filled;
		color=lightgrey;
		node [style=filled,color=white];
		a0 -> a1 -> a2 -> a3;
		label = \"process #1\";
	}

	subgraph cluster_1 {
		node [style=filled];
		b0 -> b1 -> b2 -> b3;
		label = \"process #2\";
		color=blue
	}
	start -> a0;
	start -> b0;
	a1 -> b3;
	b2 -> a3;
	a3 -> a0;
	a3 -> end;
	b3 -> end;

	start [shape=Mdiamond];
	end [shape=Msquare];
}\n"

(* Test: Graphviz.lns *)
test Graphviz.lns get cluster =
  { "digraph" = "G"
    { "subgraph" = "cluster_0"
      { "style" = "filled" }
      { "color" = "lightgrey" }
      { "@node"
        { "style" = "filled" }
        { "color" = "white" } }
      { "link"
        { "node" = "a0" }
        { "link_type" = "->" }
        { "node" = "a1" }
        { "link_type" = "->" }
        { "node" = "a2" }
        { "link_type" = "->" }
        { "node" = "a3" } }
      { "label" = "\"process #1\"" } }
    {  }
    { "subgraph" = "cluster_1"
      { "@node"
        { "style" = "filled" } }
      { "link"
        { "node" = "b0" }
        { "link_type" = "->" }
        { "node" = "b1" }
        { "link_type" = "->" }
        { "node" = "b2" }
        { "link_type" = "->" }
        { "node" = "b3" } }
      { "label" = "\"process #2\"" }
      { "color" = "blue" } }
    { "link"
      { "node" = "start" }
      { "link_type" = "->" }
      { "node" = "a0" } }
    { "link"
      { "node" = "start" }
      { "link_type" = "->" }
      { "node" = "b0" } }
    { "link"
      { "node" = "a1" }
      { "link_type" = "->" }
      { "node" = "b3" } }
    { "link"
      { "node" = "b2" }
      { "link_type" = "->" }
      { "node" = "a3" } }
    { "link"
      { "node" = "a3" }
      { "link_type" = "->" }
      { "node" = "a0" } }
    { "link"
      { "node" = "a3" }
      { "link_type" = "->" }
      { "node" = "end" } }
    { "link"
      { "node" = "b3" }
      { "link_type" = "->" }
      { "node" = "end" } }
    {  }
    { "node" = "start"
      { "shape" = "Mdiamond" } }
    { "node" = "end"
      { "shape" = "Msquare" } } } 

(* Variable: fdpclust
     From http://www.graphviz.org/content/fdpclust *)
let fdpclust = "graph G {
  e
  subgraph clusterA {
    a -- b;
    subgraph clusterC {
      C -- D;
    }
  }
  subgraph clusterB {
    d -- f
  }
  d -- D
  e -- clusterB
  clusterC -- clusterB
}\n"

(* Test: Graphviz.lns *)
test Graphviz.lns get fdpclust =
  { "graph" = "G"
    { "node" = "e" }
    { "subgraph" = "clusterA"
      { "link"
        { "node" = "a" }
        { "link_type" = "--" }
        { "node" = "b" } }
      { "subgraph" = "clusterC"
        { "link"
          { "node" = "C" }
          { "link_type" = "--" }
          { "node" = "D" } } } }
    { "subgraph" = "clusterB"
      { "link"
        { "node" = "d" }
        { "link_type" = "--" }
        { "node" = "f" } } }
    { "link"
      { "node" = "d" }
      { "link_type" = "--" }
      { "node" = "D" } }
    { "link"
      { "node" = "e" }
      { "link_type" = "--" }
      { "node" = "clusterB" } }
    { "link"
      { "node" = "clusterC" }
      { "link_type" = "--" }
      { "node" = "clusterB" } } } 

(* Test: Graphviz.nodelist *)
test Graphviz.nodelist get "LR_0 LR_3 LR_4 LR_8" =
  { "nodelist"
    { "node" = "LR_0" }
    { "node" = "LR_3" }
    { "node" = "LR_4" }
    { "node" = "LR_8" } } 



(* Variable: fsm
     From http://www.graphviz.org/content/fsm *)
let fsm = "digraph finite_state_machine {
	rankdir=LR;
	size=\"8,5\"
	node [shape = doublecircle]; LR_0 LR_3 LR_4 LR_8;
	node [shape = circle];
	LR_0 -> LR_2 [ label = \"SS(B)\" ];
	LR_0 -> LR_1 [ label = \"SS(S)\" ];
	LR_1 -> LR_3 [ label = \"S($end)\" ];
	LR_2 -> LR_6 [ label = \"SS(b)\" ];
	LR_2 -> LR_5 [ label = \"SS(a)\" ];
	LR_2 -> LR_4 [ label = \"S(A)\" ];
	LR_5 -> LR_7 [ label = \"S(b)\" ];
	LR_5 -> LR_5 [ label = \"S(a)\" ];
	LR_6 -> LR_6 [ label = \"S(b)\" ];
	LR_6 -> LR_5 [ label = \"S(a)\" ];
	LR_7 -> LR_8 [ label = \"S(b)\" ];
	LR_7 -> LR_5 [ label = \"S(a)\" ];
	LR_8 -> LR_6 [ label = \"S(b)\" ];
	LR_8 -> LR_5 [ label = \"S(a)\" ];
}\n"

(* Test: Graphviz.lns *)
test Graphviz.lns get fsm =
  { "digraph" = "finite_state_machine"
    { "rankdir" = "LR" }
    { "size" = "\"8,5\"" }
    { "@node"
      { "shape" = "doublecircle" } }
    { "nodelist"
      { "node" = "LR_0" }
      { "node" = "LR_3" }
      { "node" = "LR_4" }
      { "node" = "LR_8" } }
    { "@node"
      { "shape" = "circle" } }
    { "link"
      { "node" = "LR_0" }
      { "link_type" = "->" }
      { "node" = "LR_2" }
      { "label" = "\"SS(B)\"" } }
    { "link"
      { "node" = "LR_0" }
      { "link_type" = "->" }
      { "node" = "LR_1" }
      { "label" = "\"SS(S)\"" } }
    { "link"
      { "node" = "LR_1" }
      { "link_type" = "->" }
      { "node" = "LR_3" }
      { "label" = "\"S($end)\"" } }
    { "link"
      { "node" = "LR_2" }
      { "link_type" = "->" }
      { "node" = "LR_6" }
      { "label" = "\"SS(b)\"" } }
    { "link"
      { "node" = "LR_2" }
      { "link_type" = "->" }
      { "node" = "LR_5" }
      { "label" = "\"SS(a)\"" } }
    { "link"
      { "node" = "LR_2" }
      { "link_type" = "->" }
      { "node" = "LR_4" }
      { "label" = "\"S(A)\"" } }
    { "link"
      { "node" = "LR_5" }
      { "link_type" = "->" }
      { "node" = "LR_7" }
      { "label" = "\"S(b)\"" } }
    { "link"
      { "node" = "LR_5" }
      { "link_type" = "->" }
      { "node" = "LR_5" }
      { "label" = "\"S(a)\"" } }
    { "link"
      { "node" = "LR_6" }
      { "link_type" = "->" }
      { "node" = "LR_6" }
      { "label" = "\"S(b)\"" } }
    { "link"
      { "node" = "LR_6" }
      { "link_type" = "->" }
      { "node" = "LR_5" }
      { "label" = "\"S(a)\"" } }
    { "link"
      { "node" = "LR_7" }
      { "link_type" = "->" }
      { "node" = "LR_8" }
      { "label" = "\"S(b)\"" } }
    { "link"
      { "node" = "LR_7" }
      { "link_type" = "->" }
      { "node" = "LR_5" }
      { "label" = "\"S(a)\"" } }
    { "link"
      { "node" = "LR_8" }
      { "link_type" = "->" }
      { "node" = "LR_6" }
      { "label" = "\"S(b)\"" } }
    { "link"
      { "node" = "LR_8" }
      { "link_type" = "->" }
      { "node" = "LR_5" }
      { "label" = "\"S(a)\"" } } }

