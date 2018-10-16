(*
	Some of the configuration snippets have been copied from the strongSwan
	source tree.
*)

module Test_Strongswan =

(* conf/strongswan.conf *)
let default = "
# strongswan.conf - strongSwan configuration file
#
# Refer to the strongswan.conf(5) manpage for details
#
# Configuration changes should be made in the included files

charon {
	load_modular = yes
	plugins {
		include strongswan.d/charon/*.conf
	}
}

include strongswan.d/*.conf
"

test Strongswan.lns get default =
  { "#comment" = "strongswan.conf - strongSwan configuration file" }
  { "#comment" = "Refer to the strongswan.conf(5) manpage for details" }
  { "#comment" = "Configuration changes should be made in the included files" }
  { "charon"
    { "load_modular" = "yes" }
    { "plugins" { "include" = "strongswan.d/charon/*.conf" } }
  }
  { "include" = "strongswan.d/*.conf" }

(* conf/strongswan.conf.5.head.in *)
let man_example = "
	a = b
	section-one {
		somevalue = asdf
		subsection {
			othervalue = xxx
		}
		# yei, a comment
		yetanother = zz
	}
	section-two {
		x = 12
	}
"

test Strongswan.lns get man_example =
  { "a" = "b" }
  { "section-one"
    { "somevalue" = "asdf" }
    { "subsection" { "othervalue" = "xxx" } }
    { "#comment" = "yei, a comment" }
    { "yetanother" = "zz" }
  }
  { "section-two" { "x" = "12" } }

test Strongswan.lns get "foo { bar = baz\n } quux {}\t#quuux\n" =
  { "foo" { "bar" = "baz" } }
  { "quux" }
  { "#comment" = "quuux" }
