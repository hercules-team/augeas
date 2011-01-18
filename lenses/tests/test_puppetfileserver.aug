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
"

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
