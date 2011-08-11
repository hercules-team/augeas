(* Tests for the PuppetFileserver module *)

module Test_puppetfileserver =

let fileserver = "# This a comment

[mount1]
  # Mount1 options
  path /etc/puppet/files/%h
  allow host.domain1.com
  allow *.domain2.com
  deny badhost.domain2.com
[mount2]
  allow *
  deny *.evil.example.com
  deny badhost.domain2.com
[mount3]
allow *   # Puppet #6026: same line comment
# And trailing whitespace
allow *    	\n"

test PuppetFileserver.lns get fileserver =
	{ "#comment" = "This a comment" }
	{ }
	{ "mount1"
		{ "#comment" = "Mount1 options" }
		{ "path" = "/etc/puppet/files/%h" }
		{ "allow" = "host.domain1.com" }
		{ "allow" = "*.domain2.com" }
		{ "deny" = "badhost.domain2.com" }
	}
	{ "mount2"
		{ "allow" = "*" }
		{ "deny" = "*.evil.example.com" }
		{ "deny" = "badhost.domain2.com" }
	}
	{ "mount3"
		{ "allow" = "*"
			{ "#comment" = "Puppet #6026: same line comment" } }
		{ "#comment" = "And trailing whitespace" }
		{ "allow" = "*" }
	}
