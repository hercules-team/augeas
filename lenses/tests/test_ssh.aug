(* Module: Test_ssh *)
module Test_ssh =

    let conf =
"# start
IdentityFile /etc/ssh/identity.asc

Host suse.cz
   ForwardAgent yes
SendEnv LC_LANG

Host *
   ForwardAgent no
ForwardX11Trusted yes

#   IdentityFile ~/.ssh/identity
	SendEnv LC_IDENTIFICATION LC_ALL LC_*
ProxyCommand ssh -q -W %h:%p gateway.example.com
RemoteForward [1.2.3.4]:20023 localhost:22
RemoteForward 2221 lhost1:22
LocalForward 3001 remotehost:3000
Ciphers aes128-ctr,aes192-ctr
MACs hmac-md5,hmac-sha1,umac-64@openssh.com
HostKeyAlgorithms ssh-ed25519-cert-v01@openssh.com,ssh-ed25519,ssh-rsa-cert-v01@openssh.com,ssh-rsa
KexAlgorithms curve25519-sha256@libssh.org,diffie-hellman-group-exchange-sha256
PubkeyAcceptedKeyTypes ssh-ed25519-cert-v01@openssh.com,ssh-ed25519,ssh-rsa-cert-v01@openssh.com,ssh-rsa
"

    test Ssh.lns get conf =
    { "#comment" = "start" }
    { "IdentityFile" = "/etc/ssh/identity.asc" }
    { }
    { "Host"	= "suse.cz"
	{ "ForwardAgent"  = "yes" }
	{ "SendEnv"
	    { "1" = "LC_LANG" } }
	{ }
    }
    { "Host"	= "*"
	{ "ForwardAgent"  = "no" }
	{ "ForwardX11Trusted"  = "yes" }
	{ }
	{ "#comment" = "IdentityFile ~/.ssh/identity" }
	{ "SendEnv"
	    { "1" = "LC_IDENTIFICATION" }
	    { "2" = "LC_ALL" }
	    { "3" = "LC_*" } }
	{ "ProxyCommand" = "ssh -q -W %h:%p gateway.example.com" }
	{ "RemoteForward"
	    { "[1.2.3.4]:20023" = "localhost:22" }
	}
	{ "RemoteForward"
	    { "2221" = "lhost1:22" }
	}
	{ "LocalForward"
	    { "3001" = "remotehost:3000" }
	}
	{ "Ciphers"
	    { "1" = "aes128-ctr" }
	    { "2" = "aes192-ctr" }
	}
	{ "MACs"
	    { "1" = "hmac-md5" }
	    { "2" = "hmac-sha1" }
	    { "3" = "umac-64@openssh.com" }
	}
    { "HostKeyAlgorithms"
      { "1" = "ssh-ed25519-cert-v01@openssh.com" }
      { "2" = "ssh-ed25519" }
      { "3" = "ssh-rsa-cert-v01@openssh.com" }
      { "4" = "ssh-rsa" }
    }
    { "KexAlgorithms"
      { "1" = "curve25519-sha256@libssh.org" }
      { "2" = "diffie-hellman-group-exchange-sha256" }
    }
    { "PubkeyAcceptedKeyTypes"
      { "1" = "ssh-ed25519-cert-v01@openssh.com" }
      { "2" = "ssh-ed25519" }
      { "3" = "ssh-rsa-cert-v01@openssh.com" }
      { "4" = "ssh-rsa" }
    }
    }

(* Test: Ssh.lns
     Proxycommand is case-insensitive *)

test Ssh.lns get "Proxycommand ssh -q test nc -q0 %h 22\n" =
  { "Proxycommand" = "ssh -q test nc -q0 %h 22" }

(* Test: Ssh.lns
     GlobalKnownHostsFile *)
test Ssh.lns get "GlobalKnownHostsFile /etc/ssh/ssh_known_hosts /etc/ssh/ssh_known_hosts2\n" =
  { "GlobalKnownHostsFile"
    { "1" = "/etc/ssh/ssh_known_hosts" }
    { "2" = "/etc/ssh/ssh_known_hosts2" }
  }

(* Keywords can be separated from their arguments with '=', too *)
test Ssh.lns get "Host mail.watzmann.net
  LocalForward=11111 mail.watzmann.net:110\n" =
  { "Host" = "mail.watzmann.net"
    { "LocalForward"
      { "11111" = "mail.watzmann.net:110" } } }

test Ssh.lns get "ForwardAgent=yes\n" =
 { "ForwardAgent" = "yes" }

test Ssh.lns get "ForwardAgent =\tyes\n" =
 { "ForwardAgent" = "yes" }
