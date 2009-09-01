module Test_inetd =

	(* The standard "parse a bucket of text" test *)
	let conf = "# Blah di blah comment

simplesrv	stream	tcp	nowait	fred	/usr/bin/simplesrv
arguserve	dgram	udp	wait	mary	/usr/bin/usenet		foo bar wombat

1234		stream	tcp	nowait	fred	/usr/bin/numbersrv

127.0.0.1:addrsrv	stream	tcp	nowait	fred	/usr/bin/addrsrv
127.0.0.1,10.0.0.1:multiaddrsrv	stream tcp nowait fred /usr/bin/multiaddrsrv
faff.fred.com:
127.0.0.1,faff.fred.com:
*:

sndbufsrv	stream	tcp,sndbuf=12k	nowait	fred	/usr/bin/sndbufsrv
rcvbufsrv	stream	tcp,rcvbuf=24k	nowait	fred	/usr/bin/rcvbufsrv
allbufsrv	stream	tcp,sndbuf=1m,rcvbuf=24k	nowait	fred /usr/bin/allbufsrv

dotgroupsrv	stream	tcp	nowait	fred.wilma	/usr/bin/dotgroupsrv
colongroupsrv	stream	tcp	nowait	fred:wilma	/usr/bin/colongroupsrv

maxsrv		stream	tcp	nowait.20	fred	/usr/bin/maxsrv
"

	test Inetd.lns get conf =
		{ "#comment" = "Blah di blah comment" }
		{}
		{ "simplesrv"
			{ "socket" = "stream" }
			{ "protocol" = "tcp" }
			{ "wait" = "nowait" }
			{ "user" = "fred" }
			{ "command" = "/usr/bin/simplesrv" }
		}
		{ "arguserve"
			{ "socket" = "dgram" }
			{ "protocol" = "udp" }
			{ "wait" = "wait" }
			{ "user" = "mary" }
			{ "command" = "/usr/bin/usenet" }
			{ "arguments"
				{ "1" = "foo" }
				{ "2" = "bar" }
				{ "3" = "wombat" }
			}
		}
		{}
		{ "1234"
			{ "socket" = "stream" }
			{ "protocol" = "tcp" }
			{ "wait" = "nowait" }
			{ "user" = "fred" }
			{ "command" = "/usr/bin/numbersrv" }
		}
		{}
		{ "addrsrv"
			{ "address"
				{ "1" = "127.0.0.1" }
			}
			{ "socket" = "stream" }
			{ "protocol" = "tcp" }
			{ "wait" = "nowait" }
			{ "user" = "fred" }
			{ "command" = "/usr/bin/addrsrv" }
		}
		{ "multiaddrsrv"
			{ "address"
				{ "1" = "127.0.0.1" }
				{ "2" = "10.0.0.1" }
			}
			{ "socket" = "stream" }
			{ "protocol" = "tcp" }
			{ "wait" = "nowait" }
			{ "user" = "fred" }
			{ "command" = "/usr/bin/multiaddrsrv" }
		}
		{ "#address"
			{ "1" = "faff.fred.com" }
		}
		{ "#address"
			{ "1" = "127.0.0.1" }
			{ "2" = "faff.fred.com" }
		}
		{ "#address"
			{ "1" = "*" }
		}
		{}
		{ "sndbufsrv"
			{ "socket" = "stream" }
			{ "protocol" = "tcp" }
			{ "sndbuf" = "12k" }
			{ "wait" = "nowait" }
			{ "user" = "fred" }
			{ "command" = "/usr/bin/sndbufsrv" }
		}
		{ "rcvbufsrv"
			{ "socket" = "stream" }
			{ "protocol" = "tcp" }
			{ "rcvbuf" = "24k" }
			{ "wait" = "nowait" }
			{ "user" = "fred" }
			{ "command" = "/usr/bin/rcvbufsrv" }
		}
		{ "allbufsrv"
			{ "socket" = "stream" }
			{ "protocol" = "tcp" }
			{ "sndbuf" = "1m" }
			{ "rcvbuf" = "24k" }
			{ "wait" = "nowait" }
			{ "user" = "fred" }
			{ "command" = "/usr/bin/allbufsrv" }
		}
		{}
		{ "dotgroupsrv"
			{ "socket" = "stream" }
			{ "protocol" = "tcp" }
			{ "wait" = "nowait" }
			{ "user" = "fred" }
			{ "group" = "wilma" }
			{ "command" = "/usr/bin/dotgroupsrv" }
		}
		{ "colongroupsrv"
			{ "socket" = "stream" }
			{ "protocol" = "tcp" }
			{ "wait" = "nowait" }
			{ "user" = "fred" }
			{ "group" = "wilma" }
			{ "command" = "/usr/bin/colongroupsrv" }
		}
		{}
		{ "maxsrv"
			{ "socket" = "stream" }
			{ "protocol" = "tcp" }
			{ "wait" = "nowait" }
			{ "max" = "20" }
			{ "user" = "fred" }
			{ "command" = "/usr/bin/maxsrv" }
		}


(**************************************************************************)

	(* Test new file creation *)

	test Inetd.lns put "" after
		set "/faffsrv/socket" "stream";
		set "/faffsrv/protocol" "tcp";
		set "/faffsrv/wait" "nowait";
		set "/faffsrv/user" "george";
		set "/faffsrv/command" "/sbin/faffsrv"
	= "faffsrv	stream	tcp	nowait	george	/sbin/faffsrv\n"
