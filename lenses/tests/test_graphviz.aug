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
    { "@default" = "node"
      { "style" = "filled" }
      { "color" = "white" } }
    { "label" = "\"process #1\"" } }

(* Test: Graphviz.lns
     Set test *)
test Graphviz.lns put entries after
   set "/graph/@default/color" "red" =
"graph A {
style=filled;
color=lightgrey;
node [style=filled,color=red];
label = \"process #1\";
}\n"

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
      { "@default" = "node"
        { "style" = "filled" }
        { "color" = "white" }
      }
      { "link"
        { "node" = "a0" }
        { "link_type" = "->" }
        { "node" = "a1" }
        { "link_type" = "->" }
        { "node" = "a2" }
        { "link_type" = "->" }
        { "node" = "a3" }
      }
      { "label" = "\"process #1\"" }
    }
    { "subgraph" = "cluster_1"
      { "@default" = "node"
        { "style" = "filled" }
      }
      { "link"
        { "node" = "b0" }
        { "link_type" = "->" }
        { "node" = "b1" }
        { "link_type" = "->" }
        { "node" = "b2" }
        { "link_type" = "->" }
        { "node" = "b3" }
      }
      { "label" = "\"process #2\"" }
      { "color" = "blue" }
    }
    { "link"
      { "node" = "start" }
      { "link_type" = "->" }
      { "node" = "a0" }
    }
    { "link"
      { "node" = "start" }
      { "link_type" = "->" }
      { "node" = "b0" }
    }
    { "link"
      { "node" = "a1" }
      { "link_type" = "->" }
      { "node" = "b3" }
    }
    { "link"
      { "node" = "b2" }
      { "link_type" = "->" }
      { "node" = "a3" }
    }
    { "link"
      { "node" = "a3" }
      { "link_type" = "->" }
      { "node" = "a0" }
    }
    { "link"
      { "node" = "a3" }
      { "link_type" = "->" }
      { "node" = "end" }
    }
    { "link"
      { "node" = "b3" }
      { "link_type" = "->" }
      { "node" = "end" }
    }
    { "nodelist"
      { "node" = "start"
        { "shape" = "Mdiamond" }
      }
    }
    { "nodelist"
      { "node" = "end"
        { "shape" = "Msquare" }
      }
    }
  }


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
    { "nodelist"
      { "node" = "e" }
    }
    { "subgraph" = "clusterA"
      { "link"
        { "node" = "a" }
        { "link_type" = "--" }
        { "node" = "b" }
      }
      { "subgraph" = "clusterC"
        { "link"
          { "node" = "C" }
          { "link_type" = "--" }
          { "node" = "D" }
        }
      }
    }
    { "subgraph" = "clusterB"
      { "link"
        { "node" = "d" }
        { "link_type" = "--" }
        { "node" = "f" }
      }
    }
    { "link"
      { "node" = "d" }
      { "link_type" = "--" }
      { "node" = "D" }
    }
    { "link"
      { "node" = "e" }
      { "link_type" = "--" }
      { "node" = "clusterB" }
    }
    { "link"
      { "node" = "clusterC" }
      { "link_type" = "--" }
      { "node" = "clusterB" }
    }
  }


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
    { "@default" = "node"
      { "shape" = "doublecircle" }
    }
    { "nodelist"
      { "node" = "LR_0" }
      { "node" = "LR_3" }
      { "node" = "LR_4" }
      { "node" = "LR_8" }
    }
    { "@default" = "node"
      { "shape" = "circle" }
    }
    { "link"
      { "node" = "LR_0" }
      { "link_type" = "->" }
      { "node" = "LR_2" }
      { "label" = "\"SS(B)\"" }
    }
    { "link"
      { "node" = "LR_0" }
      { "link_type" = "->" }
      { "node" = "LR_1" }
      { "label" = "\"SS(S)\"" }
    }
    { "link"
      { "node" = "LR_1" }
      { "link_type" = "->" }
      { "node" = "LR_3" }
      { "label" = "\"S($end)\"" }
    }
    { "link"
      { "node" = "LR_2" }
      { "link_type" = "->" }
      { "node" = "LR_6" }
      { "label" = "\"SS(b)\"" }
    }
    { "link"
      { "node" = "LR_2" }
      { "link_type" = "->" }
      { "node" = "LR_5" }
      { "label" = "\"SS(a)\"" }
    }
    { "link"
      { "node" = "LR_2" }
      { "link_type" = "->" }
      { "node" = "LR_4" }
      { "label" = "\"S(A)\"" }
    }
    { "link"
      { "node" = "LR_5" }
      { "link_type" = "->" }
      { "node" = "LR_7" }
      { "label" = "\"S(b)\"" }
    }
    { "link"
      { "node" = "LR_5" }
      { "link_type" = "->" }
      { "node" = "LR_5" }
      { "label" = "\"S(a)\"" }
    }
    { "link"
      { "node" = "LR_6" }
      { "link_type" = "->" }
      { "node" = "LR_6" }
      { "label" = "\"S(b)\"" }
    }
    { "link"
      { "node" = "LR_6" }
      { "link_type" = "->" }
      { "node" = "LR_5" }
      { "label" = "\"S(a)\"" }
    }
    { "link"
      { "node" = "LR_7" }
      { "link_type" = "->" }
      { "node" = "LR_8" }
      { "label" = "\"S(b)\"" }
    }
    { "link"
      { "node" = "LR_7" }
      { "link_type" = "->" }
      { "node" = "LR_5" }
      { "label" = "\"S(a)\"" }
    }
    { "link"
      { "node" = "LR_8" }
      { "link_type" = "->" }
      { "node" = "LR_6" }
      { "label" = "\"S(b)\"" }
    }
    { "link"
      { "node" = "LR_8" }
      { "link_type" = "->" }
      { "node" = "LR_5" }
      { "label" = "\"S(a)\"" }
    }
  }

(* Variable: crazy
    From http://www.graphviz.org/content/crazy *)
let crazy = "digraph \"unix\" {
	graph [	fontname = \"Helvetica-Oblique\",
		fontsize = 36,
		label = \"\n\n\n\nObject Oriented Graphs\nStephen North, 3/19/93\",
		size = \"6,6\" ];
	node [	shape = polygon,
		sides = 4,
		distortion = \"0.0\",
		orientation = \"0.0\",
		skew = \"0.0\",
		color = white,
		style = filled,
		fontname = \"Helvetica-Outline\" ];
	\"5th Edition\" [sides=9, distortion=\"0.936354\", orientation=28, skew=\"-0.126818\", color=salmon2];
	\"6th Edition\" [sides=5, distortion=\"0.238792\", orientation=11, skew=\"0.995935\", color=deepskyblue];
	\"PWB 1.0\" [sides=8, distortion=\"0.019636\", orientation=79, skew=\"-0.440424\", color=goldenrod2];
	LSX [sides=9, distortion=\"-0.698271\", orientation=22, skew=\"-0.195492\", color=burlywood2];
	\"1 BSD\" [sides=7, distortion=\"0.265084\", orientation=26, skew=\"0.403659\", color=gold1];
	\"Mini Unix\" [distortion=\"0.039386\", orientation=2, skew=\"-0.461120\", color=greenyellow];
	Wollongong [sides=5, distortion=\"0.228564\", orientation=63, skew=\"-0.062846\", color=darkseagreen];
	Interdata [distortion=\"0.624013\", orientation=56, skew=\"0.101396\", color=dodgerblue1];
	\"Unix/TS 3.0\" [sides=8, distortion=\"0.731383\", orientation=43, skew=\"-0.824612\", color=thistle2];
	\"PWB 2.0\" [sides=6, distortion=\"0.592100\", orientation=34, skew=\"-0.719269\", color=darkolivegreen3];
	\"7th Edition\" [sides=10, distortion=\"0.298417\", orientation=65, skew=\"0.310367\", color=chocolate];
	\"8th Edition\" [distortion=\"-0.997093\", orientation=50, skew=\"-0.061117\", color=turquoise3];
	\"32V\" [sides=7, distortion=\"0.878516\", orientation=19, skew=\"0.592905\", color=steelblue3];
	V7M [sides=10, distortion=\"-0.960249\", orientation=32, skew=\"0.460424\", color=navy];
	\"Ultrix-11\" [sides=10, distortion=\"-0.633186\", orientation=10, skew=\"0.333125\", color=darkseagreen4];
	Xenix [sides=8, distortion=\"-0.337997\", orientation=52, skew=\"-0.760726\", color=coral];
	\"UniPlus+\" [sides=7, distortion=\"0.788483\", orientation=39, skew=\"-0.526284\", color=darkolivegreen3];
	\"9th Edition\" [sides=7, distortion=\"0.138690\", orientation=55, skew=\"0.554049\", color=coral3];
	\"2 BSD\" [sides=7, distortion=\"-0.010661\", orientation=84, skew=\"0.179249\", color=blanchedalmond];
	\"2.8 BSD\" [distortion=\"-0.239422\", orientation=44, skew=\"0.053841\", color=lightskyblue1];
	\"2.9 BSD\" [distortion=\"-0.843381\", orientation=70, skew=\"-0.601395\", color=aquamarine2];
	\"3 BSD\" [sides=10, distortion=\"0.251820\", orientation=18, skew=\"-0.530618\", color=lemonchiffon];
	\"4 BSD\" [sides=5, distortion=\"-0.772300\", orientation=24, skew=\"-0.028475\", color=darkorange1];
	\"4.1 BSD\" [distortion=\"-0.226170\", orientation=38, skew=\"0.504053\", color=lightyellow1];
	\"4.2 BSD\" [sides=10, distortion=\"-0.807349\", orientation=50, skew=\"-0.908842\", color=darkorchid4];
	\"4.3 BSD\" [sides=10, distortion=\"-0.030619\", orientation=76, skew=\"0.985021\", color=lemonchiffon2];
	\"Ultrix-32\" [distortion=\"-0.644209\", orientation=21, skew=\"0.307836\", color=goldenrod3];
	\"PWB 1.2\" [sides=7, distortion=\"0.640971\", orientation=84, skew=\"-0.768455\", color=cyan];
	\"USG 1.0\" [distortion=\"0.758942\", orientation=42, skew=\"0.039886\", color=blue];
	\"CB Unix 1\" [sides=9, distortion=\"-0.348692\", orientation=42, skew=\"0.767058\", color=firebrick];
	\"USG 2.0\" [distortion=\"0.748625\", orientation=74, skew=\"-0.647656\", color=chartreuse4];
	\"CB Unix 2\" [sides=10, distortion=\"0.851818\", orientation=32, skew=\"-0.020120\", color=greenyellow];
	\"CB Unix 3\" [sides=10, distortion=\"0.992237\", orientation=29, skew=\"0.256102\", color=bisque4];
	\"Unix/TS++\" [sides=6, distortion=\"0.545461\", orientation=16, skew=\"0.313589\", color=mistyrose2];
	\"PDP-11 Sys V\" [sides=9, distortion=\"-0.267769\", orientation=40, skew=\"0.271226\", color=cadetblue1];
	\"USG 3.0\" [distortion=\"-0.848455\", orientation=44, skew=\"0.267152\", color=bisque2];
	\"Unix/TS 1.0\" [distortion=\"0.305594\", orientation=75, skew=\"0.070516\", color=orangered];
	\"TS 4.0\" [sides=10, distortion=\"-0.641701\", orientation=50, skew=\"-0.952502\", color=crimson];
	\"System V.0\" [sides=9, distortion=\"0.021556\", orientation=26, skew=\"-0.729938\", color=darkorange1];
	\"System V.2\" [sides=6, distortion=\"0.985153\", orientation=33, skew=\"-0.399752\", color=darkolivegreen4];
	\"System V.3\" [sides=7, distortion=\"-0.687574\", orientation=58, skew=\"-0.180116\", color=lightsteelblue1];
	\"5th Edition\" -> \"6th Edition\";
	\"5th Edition\" -> \"PWB 1.0\";
	\"6th Edition\" -> LSX;
	\"6th Edition\" -> \"1 BSD\";
	\"6th Edition\" -> \"Mini Unix\";
	\"6th Edition\" -> Wollongong;
	\"6th Edition\" -> Interdata;
	Interdata -> \"Unix/TS 3.0\";
	Interdata -> \"PWB 2.0\";
	Interdata -> \"7th Edition\";
	\"7th Edition\" -> \"8th Edition\";
	\"7th Edition\" -> \"32V\";
	\"7th Edition\" -> V7M;
	\"7th Edition\" -> \"Ultrix-11\";
	\"7th Edition\" -> Xenix;
	\"7th Edition\" -> \"UniPlus+\";
	V7M -> \"Ultrix-11\";
	\"8th Edition\" -> \"9th Edition\";
	\"1 BSD\" -> \"2 BSD\";
	\"2 BSD\" -> \"2.8 BSD\";
	\"2.8 BSD\" -> \"Ultrix-11\";
	\"2.8 BSD\" -> \"2.9 BSD\";
	\"32V\" -> \"3 BSD\";
	\"3 BSD\" -> \"4 BSD\";
	\"4 BSD\" -> \"4.1 BSD\";
	\"4.1 BSD\" -> \"4.2 BSD\";
	\"4.1 BSD\" -> \"2.8 BSD\";
	\"4.1 BSD\" -> \"8th Edition\";
	\"4.2 BSD\" -> \"4.3 BSD\";
	\"4.2 BSD\" -> \"Ultrix-32\";
	\"PWB 1.0\" -> \"PWB 1.2\";
	\"PWB 1.0\" -> \"USG 1.0\";
	\"PWB 1.2\" -> \"PWB 2.0\";
	\"USG 1.0\" -> \"CB Unix 1\";
	\"USG 1.0\" -> \"USG 2.0\";
	\"CB Unix 1\" -> \"CB Unix 2\";
	\"CB Unix 2\" -> \"CB Unix 3\";
	\"CB Unix 3\" -> \"Unix/TS++\";
	\"CB Unix 3\" -> \"PDP-11 Sys V\";
	\"USG 2.0\" -> \"USG 3.0\";
	\"USG 3.0\" -> \"Unix/TS 3.0\";
	\"PWB 2.0\" -> \"Unix/TS 3.0\";
	\"Unix/TS 1.0\" -> \"Unix/TS 3.0\";
	\"Unix/TS 3.0\" -> \"TS 4.0\";
	\"Unix/TS++\" -> \"TS 4.0\";
	\"CB Unix 3\" -> \"TS 4.0\";
	\"TS 4.0\" -> \"System V.0\";
	\"System V.0\" -> \"System V.2\";
	\"System V.2\" -> \"System V.3\";
}\n"

(* Test: Graphviz.lns *)
test Graphviz.lns get crazy =
  { "digraph" = "\"unix\""
    { "@default" = "graph"
      { "fontname" = "\"Helvetica-Oblique\"" }
      { "fontsize" = "36" }
      { "label" = "\"\n\n\n\nObject Oriented Graphs\nStephen North, 3/19/93\"" }
      { "size" = "\"6,6\"" }
    }
    { "@default" = "node"
      { "shape" = "polygon" }
      { "sides" = "4" }
      { "distortion" = "\"0.0\"" }
      { "orientation" = "\"0.0\"" }
      { "skew" = "\"0.0\"" }
      { "color" = "white" }
      { "style" = "filled" }
      { "fontname" = "\"Helvetica-Outline\"" }
    }
    { "nodelist"
      { "node" = "\"5th Edition\""
        { "sides" = "9" }
        { "distortion" = "\"0.936354\"" }
        { "orientation" = "28" }
        { "skew" = "\"-0.126818\"" }
        { "color" = "salmon2" }
      }
    }
    { "nodelist"
      { "node" = "\"6th Edition\""
        { "sides" = "5" }
        { "distortion" = "\"0.238792\"" }
        { "orientation" = "11" }
        { "skew" = "\"0.995935\"" }
        { "color" = "deepskyblue" }
      }
    }
    { "nodelist"
      { "node" = "\"PWB 1.0\""
        { "sides" = "8" }
        { "distortion" = "\"0.019636\"" }
        { "orientation" = "79" }
        { "skew" = "\"-0.440424\"" }
        { "color" = "goldenrod2" }
      }
    }
    { "nodelist"
      { "node" = "LSX"
        { "sides" = "9" }
        { "distortion" = "\"-0.698271\"" }
        { "orientation" = "22" }
        { "skew" = "\"-0.195492\"" }
        { "color" = "burlywood2" }
      }
    }
    { "nodelist"
      { "node" = "\"1 BSD\""
        { "sides" = "7" }
        { "distortion" = "\"0.265084\"" }
        { "orientation" = "26" }
        { "skew" = "\"0.403659\"" }
        { "color" = "gold1" }
      }
    }
    { "nodelist"
      { "node" = "\"Mini Unix\""
        { "distortion" = "\"0.039386\"" }
        { "orientation" = "2" }
        { "skew" = "\"-0.461120\"" }
        { "color" = "greenyellow" }
      }
    }
    { "nodelist"
      { "node" = "Wollongong"
        { "sides" = "5" }
        { "distortion" = "\"0.228564\"" }
        { "orientation" = "63" }
        { "skew" = "\"-0.062846\"" }
        { "color" = "darkseagreen" }
      }
    }
    { "nodelist"
      { "node" = "Interdata"
        { "distortion" = "\"0.624013\"" }
        { "orientation" = "56" }
        { "skew" = "\"0.101396\"" }
        { "color" = "dodgerblue1" }
      }
    }
    { "nodelist"
      { "node" = "\"Unix/TS 3.0\""
        { "sides" = "8" }
        { "distortion" = "\"0.731383\"" }
        { "orientation" = "43" }
        { "skew" = "\"-0.824612\"" }
        { "color" = "thistle2" }
      }
    }
    { "nodelist"
      { "node" = "\"PWB 2.0\""
        { "sides" = "6" }
        { "distortion" = "\"0.592100\"" }
        { "orientation" = "34" }
        { "skew" = "\"-0.719269\"" }
        { "color" = "darkolivegreen3" }
      }
    }
    { "nodelist"
      { "node" = "\"7th Edition\""
        { "sides" = "10" }
        { "distortion" = "\"0.298417\"" }
        { "orientation" = "65" }
        { "skew" = "\"0.310367\"" }
        { "color" = "chocolate" }
      }
    }
    { "nodelist"
      { "node" = "\"8th Edition\""
        { "distortion" = "\"-0.997093\"" }
        { "orientation" = "50" }
        { "skew" = "\"-0.061117\"" }
        { "color" = "turquoise3" }
      }
    }
    { "nodelist"
      { "node" = "\"32V\""
        { "sides" = "7" }
        { "distortion" = "\"0.878516\"" }
        { "orientation" = "19" }
        { "skew" = "\"0.592905\"" }
        { "color" = "steelblue3" }
      }
    }
    { "nodelist"
      { "node" = "V7M"
        { "sides" = "10" }
        { "distortion" = "\"-0.960249\"" }
        { "orientation" = "32" }
        { "skew" = "\"0.460424\"" }
        { "color" = "navy" }
      }
    }
    { "nodelist"
      { "node" = "\"Ultrix-11\""
        { "sides" = "10" }
        { "distortion" = "\"-0.633186\"" }
        { "orientation" = "10" }
        { "skew" = "\"0.333125\"" }
        { "color" = "darkseagreen4" }
      }
    }
    { "nodelist"
      { "node" = "Xenix"
        { "sides" = "8" }
        { "distortion" = "\"-0.337997\"" }
        { "orientation" = "52" }
        { "skew" = "\"-0.760726\"" }
        { "color" = "coral" }
      }
    }
    { "nodelist"
      { "node" = "\"UniPlus+\""
        { "sides" = "7" }
        { "distortion" = "\"0.788483\"" }
        { "orientation" = "39" }
        { "skew" = "\"-0.526284\"" }
        { "color" = "darkolivegreen3" }
      }
    }
    { "nodelist"
      { "node" = "\"9th Edition\""
        { "sides" = "7" }
        { "distortion" = "\"0.138690\"" }
        { "orientation" = "55" }
        { "skew" = "\"0.554049\"" }
        { "color" = "coral3" }
      }
    }
    { "nodelist"
      { "node" = "\"2 BSD\""
        { "sides" = "7" }
        { "distortion" = "\"-0.010661\"" }
        { "orientation" = "84" }
        { "skew" = "\"0.179249\"" }
        { "color" = "blanchedalmond" }
      }
    }
    { "nodelist"
      { "node" = "\"2.8 BSD\""
        { "distortion" = "\"-0.239422\"" }
        { "orientation" = "44" }
        { "skew" = "\"0.053841\"" }
        { "color" = "lightskyblue1" }
      }
    }
    { "nodelist"
      { "node" = "\"2.9 BSD\""
        { "distortion" = "\"-0.843381\"" }
        { "orientation" = "70" }
        { "skew" = "\"-0.601395\"" }
        { "color" = "aquamarine2" }
      }
    }
    { "nodelist"
      { "node" = "\"3 BSD\""
        { "sides" = "10" }
        { "distortion" = "\"0.251820\"" }
        { "orientation" = "18" }
        { "skew" = "\"-0.530618\"" }
        { "color" = "lemonchiffon" }
      }
    }
    { "nodelist"
      { "node" = "\"4 BSD\""
        { "sides" = "5" }
        { "distortion" = "\"-0.772300\"" }
        { "orientation" = "24" }
        { "skew" = "\"-0.028475\"" }
        { "color" = "darkorange1" }
      }
    }
    { "nodelist"
      { "node" = "\"4.1 BSD\""
        { "distortion" = "\"-0.226170\"" }
        { "orientation" = "38" }
        { "skew" = "\"0.504053\"" }
        { "color" = "lightyellow1" }
      }
    }
    { "nodelist"
      { "node" = "\"4.2 BSD\""
        { "sides" = "10" }
        { "distortion" = "\"-0.807349\"" }
        { "orientation" = "50" }
        { "skew" = "\"-0.908842\"" }
        { "color" = "darkorchid4" }
      }
    }
    { "nodelist"
      { "node" = "\"4.3 BSD\""
        { "sides" = "10" }
        { "distortion" = "\"-0.030619\"" }
        { "orientation" = "76" }
        { "skew" = "\"0.985021\"" }
        { "color" = "lemonchiffon2" }
      }
    }
    { "nodelist"
      { "node" = "\"Ultrix-32\""
        { "distortion" = "\"-0.644209\"" }
        { "orientation" = "21" }
        { "skew" = "\"0.307836\"" }
        { "color" = "goldenrod3" }
      }
    }
    { "nodelist"
      { "node" = "\"PWB 1.2\""
        { "sides" = "7" }
        { "distortion" = "\"0.640971\"" }
        { "orientation" = "84" }
        { "skew" = "\"-0.768455\"" }
        { "color" = "cyan" }
      }
    }
    { "nodelist"
      { "node" = "\"USG 1.0\""
        { "distortion" = "\"0.758942\"" }
        { "orientation" = "42" }
        { "skew" = "\"0.039886\"" }
        { "color" = "blue" }
      }
    }
    { "nodelist"
      { "node" = "\"CB Unix 1\""
        { "sides" = "9" }
        { "distortion" = "\"-0.348692\"" }
        { "orientation" = "42" }
        { "skew" = "\"0.767058\"" }
        { "color" = "firebrick" }
      }
    }
    { "nodelist"
      { "node" = "\"USG 2.0\""
        { "distortion" = "\"0.748625\"" }
        { "orientation" = "74" }
        { "skew" = "\"-0.647656\"" }
        { "color" = "chartreuse4" }
      }
    }
    { "nodelist"
      { "node" = "\"CB Unix 2\""
        { "sides" = "10" }
        { "distortion" = "\"0.851818\"" }
        { "orientation" = "32" }
        { "skew" = "\"-0.020120\"" }
        { "color" = "greenyellow" }
      }
    }
    { "nodelist"
      { "node" = "\"CB Unix 3\""
        { "sides" = "10" }
        { "distortion" = "\"0.992237\"" }
        { "orientation" = "29" }
        { "skew" = "\"0.256102\"" }
        { "color" = "bisque4" }
      }
    }
    { "nodelist"
      { "node" = "\"Unix/TS++\""
        { "sides" = "6" }
        { "distortion" = "\"0.545461\"" }
        { "orientation" = "16" }
        { "skew" = "\"0.313589\"" }
        { "color" = "mistyrose2" }
      }
    }
    { "nodelist"
      { "node" = "\"PDP-11 Sys V\""
        { "sides" = "9" }
        { "distortion" = "\"-0.267769\"" }
        { "orientation" = "40" }
        { "skew" = "\"0.271226\"" }
        { "color" = "cadetblue1" }
      }
    }
    { "nodelist"
      { "node" = "\"USG 3.0\""
        { "distortion" = "\"-0.848455\"" }
        { "orientation" = "44" }
        { "skew" = "\"0.267152\"" }
        { "color" = "bisque2" }
      }
    }
    { "nodelist"
      { "node" = "\"Unix/TS 1.0\""
        { "distortion" = "\"0.305594\"" }
        { "orientation" = "75" }
        { "skew" = "\"0.070516\"" }
        { "color" = "orangered" }
      }
    }
    { "nodelist"
      { "node" = "\"TS 4.0\""
        { "sides" = "10" }
        { "distortion" = "\"-0.641701\"" }
        { "orientation" = "50" }
        { "skew" = "\"-0.952502\"" }
        { "color" = "crimson" }
      }
    }
    { "nodelist"
      { "node" = "\"System V.0\""
        { "sides" = "9" }
        { "distortion" = "\"0.021556\"" }
        { "orientation" = "26" }
        { "skew" = "\"-0.729938\"" }
        { "color" = "darkorange1" }
      }
    }
    { "nodelist"
      { "node" = "\"System V.2\""
        { "sides" = "6" }
        { "distortion" = "\"0.985153\"" }
        { "orientation" = "33" }
        { "skew" = "\"-0.399752\"" }
        { "color" = "darkolivegreen4" }
      }
    }
    { "nodelist"
      { "node" = "\"System V.3\""
        { "sides" = "7" }
        { "distortion" = "\"-0.687574\"" }
        { "orientation" = "58" }
        { "skew" = "\"-0.180116\"" }
        { "color" = "lightsteelblue1" }
      }
    }
    { "link"
      { "node" = "\"5th Edition\"" }
      { "link_type" = "->" }
      { "node" = "\"6th Edition\"" }
    }
    { "link"
      { "node" = "\"5th Edition\"" }
      { "link_type" = "->" }
      { "node" = "\"PWB 1.0\"" }
    }
    { "link"
      { "node" = "\"6th Edition\"" }
      { "link_type" = "->" }
      { "node" = "LSX" }
    }
    { "link"
      { "node" = "\"6th Edition\"" }
      { "link_type" = "->" }
      { "node" = "\"1 BSD\"" }
    }
    { "link"
      { "node" = "\"6th Edition\"" }
      { "link_type" = "->" }
      { "node" = "\"Mini Unix\"" }
    }
    { "link"
      { "node" = "\"6th Edition\"" }
      { "link_type" = "->" }
      { "node" = "Wollongong" }
    }
    { "link"
      { "node" = "\"6th Edition\"" }
      { "link_type" = "->" }
      { "node" = "Interdata" }
    }
    { "link"
      { "node" = "Interdata" }
      { "link_type" = "->" }
      { "node" = "\"Unix/TS 3.0\"" }
    }
    { "link"
      { "node" = "Interdata" }
      { "link_type" = "->" }
      { "node" = "\"PWB 2.0\"" }
    }
    { "link"
      { "node" = "Interdata" }
      { "link_type" = "->" }
      { "node" = "\"7th Edition\"" }
    }
    { "link"
      { "node" = "\"7th Edition\"" }
      { "link_type" = "->" }
      { "node" = "\"8th Edition\"" }
    }
    { "link"
      { "node" = "\"7th Edition\"" }
      { "link_type" = "->" }
      { "node" = "\"32V\"" }
    }
    { "link"
      { "node" = "\"7th Edition\"" }
      { "link_type" = "->" }
      { "node" = "V7M" }
    }
    { "link"
      { "node" = "\"7th Edition\"" }
      { "link_type" = "->" }
      { "node" = "\"Ultrix-11\"" }
    }
    { "link"
      { "node" = "\"7th Edition\"" }
      { "link_type" = "->" }
      { "node" = "Xenix" }
    }
    { "link"
      { "node" = "\"7th Edition\"" }
      { "link_type" = "->" }
      { "node" = "\"UniPlus+\"" }
    }
    { "link"
      { "node" = "V7M" }
      { "link_type" = "->" }
      { "node" = "\"Ultrix-11\"" }
    }
    { "link"
      { "node" = "\"8th Edition\"" }
      { "link_type" = "->" }
      { "node" = "\"9th Edition\"" }
    }
    { "link"
      { "node" = "\"1 BSD\"" }
      { "link_type" = "->" }
      { "node" = "\"2 BSD\"" }
    }
    { "link"
      { "node" = "\"2 BSD\"" }
      { "link_type" = "->" }
      { "node" = "\"2.8 BSD\"" }
    }
    { "link"
      { "node" = "\"2.8 BSD\"" }
      { "link_type" = "->" }
      { "node" = "\"Ultrix-11\"" }
    }
    { "link"
      { "node" = "\"2.8 BSD\"" }
      { "link_type" = "->" }
      { "node" = "\"2.9 BSD\"" }
    }
    { "link"
      { "node" = "\"32V\"" }
      { "link_type" = "->" }
      { "node" = "\"3 BSD\"" }
    }
    { "link"
      { "node" = "\"3 BSD\"" }
      { "link_type" = "->" }
      { "node" = "\"4 BSD\"" }
    }
    { "link"
      { "node" = "\"4 BSD\"" }
      { "link_type" = "->" }
      { "node" = "\"4.1 BSD\"" }
    }
    { "link"
      { "node" = "\"4.1 BSD\"" }
      { "link_type" = "->" }
      { "node" = "\"4.2 BSD\"" }
    }
    { "link"
      { "node" = "\"4.1 BSD\"" }
      { "link_type" = "->" }
      { "node" = "\"2.8 BSD\"" }
    }
    { "link"
      { "node" = "\"4.1 BSD\"" }
      { "link_type" = "->" }
      { "node" = "\"8th Edition\"" }
    }
    { "link"
      { "node" = "\"4.2 BSD\"" }
      { "link_type" = "->" }
      { "node" = "\"4.3 BSD\"" }
    }
    { "link"
      { "node" = "\"4.2 BSD\"" }
      { "link_type" = "->" }
      { "node" = "\"Ultrix-32\"" }
    }
    { "link"
      { "node" = "\"PWB 1.0\"" }
      { "link_type" = "->" }
      { "node" = "\"PWB 1.2\"" }
    }
    { "link"
      { "node" = "\"PWB 1.0\"" }
      { "link_type" = "->" }
      { "node" = "\"USG 1.0\"" }
    }
    { "link"
      { "node" = "\"PWB 1.2\"" }
      { "link_type" = "->" }
      { "node" = "\"PWB 2.0\"" }
    }
    { "link"
      { "node" = "\"USG 1.0\"" }
      { "link_type" = "->" }
      { "node" = "\"CB Unix 1\"" }
    }
    { "link"
      { "node" = "\"USG 1.0\"" }
      { "link_type" = "->" }
      { "node" = "\"USG 2.0\"" }
    }
    { "link"
      { "node" = "\"CB Unix 1\"" }
      { "link_type" = "->" }
      { "node" = "\"CB Unix 2\"" }
    }
    { "link"
      { "node" = "\"CB Unix 2\"" }
      { "link_type" = "->" }
      { "node" = "\"CB Unix 3\"" }
    }
    { "link"
      { "node" = "\"CB Unix 3\"" }
      { "link_type" = "->" }
      { "node" = "\"Unix/TS++\"" }
    }
    { "link"
      { "node" = "\"CB Unix 3\"" }
      { "link_type" = "->" }
      { "node" = "\"PDP-11 Sys V\"" }
    }
    { "link"
      { "node" = "\"USG 2.0\"" }
      { "link_type" = "->" }
      { "node" = "\"USG 3.0\"" }
    }
    { "link"
      { "node" = "\"USG 3.0\"" }
      { "link_type" = "->" }
      { "node" = "\"Unix/TS 3.0\"" }
    }
    { "link"
      { "node" = "\"PWB 2.0\"" }
      { "link_type" = "->" }
      { "node" = "\"Unix/TS 3.0\"" }
    }
    { "link"
      { "node" = "\"Unix/TS 1.0\"" }
      { "link_type" = "->" }
      { "node" = "\"Unix/TS 3.0\"" }
    }
    { "link"
      { "node" = "\"Unix/TS 3.0\"" }
      { "link_type" = "->" }
      { "node" = "\"TS 4.0\"" }
    }
    { "link"
      { "node" = "\"Unix/TS++\"" }
      { "link_type" = "->" }
      { "node" = "\"TS 4.0\"" }
    }
    { "link"
      { "node" = "\"CB Unix 3\"" }
      { "link_type" = "->" }
      { "node" = "\"TS 4.0\"" }
    }
    { "link"
      { "node" = "\"TS 4.0\"" }
      { "link_type" = "->" }
      { "node" = "\"System V.0\"" }
    }
    { "link"
      { "node" = "\"System V.0\"" }
      { "link_type" = "->" }
      { "node" = "\"System V.2\"" }
    }
    { "link"
      { "node" = "\"System V.2\"" }
      { "link_type" = "->" }
      { "node" = "\"System V.3\"" }
    }
  }

