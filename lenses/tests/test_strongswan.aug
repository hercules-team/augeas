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


let connection = "

connections {
	foo {
		pools = bar, baz
		proposals = aes256gcm16-aes128gcm16-ecp512, aes256-sha256-sha1-ecp256-modp4096-modp2048, 3des-md5-modp768
	}
	children {
		bar {
			esp_proposals = aes128-sha256-sha1,3des-md5
		}
	}
}
"

test Strongswan.lns get connection =
  { "connections"
    { "foo"
      { "#list" = "pools"
        { "1" = "bar" }
        { "2" = "baz" }
      }
      { "#proposals" = "proposals"
        { "1"
          { "1" = "aes256gcm16" }
          { "2" = "aes128gcm16" }
          { "3" = "ecp512" }
        }
        { "2"
          { "1" = "aes256" }
          { "2" = "sha256" }
          { "3" = "sha1" }
          { "4" = "ecp256" }
          { "5" = "modp4096" }
          { "6" = "modp2048" }
        }
        { "3"
          { "1" = "3des" }
          { "2" = "md5" }
          { "3" = "modp768" }
        }
      }
    }
    { "children"
      { "bar"
        { "#proposals" = "esp_proposals"
          { "1"
            { "1" = "aes128" }
            { "2" = "sha256" }
            { "3" = "sha1" }
          }
          { "2"
            { "1" = "3des" }
            { "2" = "md5" }
          }
        }
      }
    }
  }
