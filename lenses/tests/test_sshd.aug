(* Module: Test_sshd *)
module Test_sshd =

  let accept_env = "Protocol 2
AcceptEnv LC_PAPER LC_NAME LC_ADDRESS LC_TELEPHONE LC_MEASUREMENT
AcceptEnv LC_IDENTIFICATION LC_ALL\n"

  test Sshd.lns get accept_env =
    { "Protocol" = "2" }
    { "AcceptEnv"
        { "1" = "LC_PAPER" }
        { "2" = "LC_NAME" }
        { "3" = "LC_ADDRESS" }
        { "4" = "LC_TELEPHONE" }
        { "5" = "LC_MEASUREMENT" } }
    { "AcceptEnv"
        { "6" = "LC_IDENTIFICATION" }
        { "7" = "LC_ALL" } }


  test Sshd.lns get "HostKey /etc/ssh/ssh_host_rsa_key
HostKey /etc/ssh/ssh_host_dsa_key\n" =
    { "HostKey" = "/etc/ssh/ssh_host_rsa_key" }
    { "HostKey" = "/etc/ssh/ssh_host_dsa_key" }


  test Sshd.lns put accept_env after
      rm "AcceptEnv";
      rm "AcceptEnv";
      set "Protocol" "1.5";
      set "X11Forwarding" "yes"
   = "Protocol 1.5\nX11Forwarding yes\n"

  test Sshd.lns get "AuthorizedKeysFile  %h/.ssh/authorized_keys\n" =
    { "AuthorizedKeysFile" = "%h/.ssh/authorized_keys" }

  test Sshd.lns get "Subsystem sftp /usr/lib/openssh/sftp-server\n" =
    { "Subsystem"
	{ "sftp" = "/usr/lib/openssh/sftp-server" } }

  test Sshd.lns get "Subsystem sftp-test /usr/lib/openssh/sftp-server\n" =
    { "Subsystem"
	{ "sftp-test" = "/usr/lib/openssh/sftp-server" } }



  let match_blocks = "X11Forwarding yes
Match User sarko Group pres.*
  Banner /etc/bienvenue.txt
  X11Forwarding no
Match User bush Group pres.* Host white.house.*
Banner /etc/welcome.txt
Match Group \"Domain users\"
  X11Forwarding yes
"
  test Sshd.lns get match_blocks =
    { "X11Forwarding" = "yes"}
      { "Match"
	  { "Condition" { "User" = "sarko"   }
	                { "Group" = "pres.*" } }
	  { "Settings"  { "Banner" = "/etc/bienvenue.txt" }
       	                { "X11Forwarding" = "no" } } }
      { "Match"
	  { "Condition" { "User" = "bush"    }
	                { "Group" = "pres.*" }
	                { "Host"  = "white.house.*" } }
	  { "Settings"  { "Banner" = "/etc/welcome.txt" } } }
      { "Match"
	  { "Condition" { "Group" = "Domain users" } }
	  { "Settings"  { "X11Forwarding" = "yes" } } }

  test Sshd.lns put match_blocks after
    insb "Subsystem" "/Match[1]";
    set "/Subsystem/sftp" "/usr/libexec/openssh/sftp-server"
  = "X11Forwarding yes
Subsystem sftp /usr/libexec/openssh/sftp-server
Match User sarko Group pres.*
  Banner /etc/bienvenue.txt
  X11Forwarding no
Match User bush Group pres.* Host white.house.*
Banner /etc/welcome.txt
Match Group \"Domain users\"
  X11Forwarding yes\n"

(* Test: Sshd.lns
   Indent when adding to a Match group *)
  test Sshd.lns put match_blocks after
    set "Match[1]/Settings/PermitRootLogin" "yes";
    set "Match[1]/Settings/#comment" "a comment" =
"X11Forwarding yes
Match User sarko Group pres.*
  Banner /etc/bienvenue.txt
  X11Forwarding no
  PermitRootLogin yes
  # a comment
Match User bush Group pres.* Host white.house.*
Banner /etc/welcome.txt
Match Group \"Domain users\"
  X11Forwarding yes\n"


(* Test: Sshd.lns
     Parse Ciphers, KexAlgorithms, HostKeyAlgorithms as lists (GH issue #69)
     Parse GSSAPIKexAlgorithms, PubkeyAcceptedKeyTypes, CASignatureAlgorithms as lists (GH PR #721) *)
test Sshd.lns get "Ciphers aes256-gcm@openssh.com,aes128-gcm@openssh.com,aes256-ctr,aes128-ctr
KexAlgorithms diffie-hellman-group-exchange-sha256,diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha1
HostKeyAlgorithms ssh-ed25519-cert-v01@openssh.com,ssh-rsa-cert-v01@openssh.com,ssh-ed25519,ssh-rsa
GSSAPIKexAlgorithms gss-curve25519-sha256-,gss-nistp256-sha256-,gss-group14-sha256-
PubkeyAcceptedKeyTypes ecdsa-sha2-nistp256,ecdsa-sha2-nistp256-cert-v01@openssh.com,ecdsa-sha2-nistp384
CASignatureAlgorithms ecdsa-sha2-nistp256,ecdsa-sha2-nistp384,ecdsa-sha2-nistp521\n" =
  { "Ciphers"
    { "1" = "aes256-gcm@openssh.com" }
    { "2" = "aes128-gcm@openssh.com" }
    { "3" = "aes256-ctr" }
    { "4" = "aes128-ctr" }
  }
  { "KexAlgorithms"
    { "1" = "diffie-hellman-group-exchange-sha256" }
    { "2" = "diffie-hellman-group14-sha1" }
    { "3" = "diffie-hellman-group-exchange-sha1" }
  }
  { "HostKeyAlgorithms"
    { "1" = "ssh-ed25519-cert-v01@openssh.com" }
    { "2" = "ssh-rsa-cert-v01@openssh.com" }
    { "3" = "ssh-ed25519" }
    { "4" = "ssh-rsa" }
  }
  { "GSSAPIKexAlgorithms"
    { "1" = "gss-curve25519-sha256-" }
    { "2" = "gss-nistp256-sha256-" }
    { "3" = "gss-group14-sha256-" }
  }
  { "PubkeyAcceptedKeyTypes"
    { "1" = "ecdsa-sha2-nistp256" }
    { "2" = "ecdsa-sha2-nistp256-cert-v01@openssh.com" }
    { "3" = "ecdsa-sha2-nistp384" }
  }
  { "CASignatureAlgorithms"
    { "1" = "ecdsa-sha2-nistp256" }
    { "2" = "ecdsa-sha2-nistp384" }
    { "3" = "ecdsa-sha2-nistp521" }
  }

(* Test: Sshd.lns
     Keys are case-insensitive *)
test Sshd.lns get "ciPheRs aes256-gcm@openssh.com,aes128-ctr
maTcH User foo
  x11forwarding no\n" =
  { "ciPheRs"
    { "1" = "aes256-gcm@openssh.com" }
    { "2" = "aes128-ctr" }
  }
  { "maTcH"
    { "Condition"
      { "User" = "foo" }
    }
    { "Settings"
      { "x11forwarding" = "no" }
    }
  }

(* Test: Sshd.lns
     Allow AllowGroups in Match groups (GH issue #75) *)
test Sshd.lns get "Match User foo
  AllowGroups users\n" =
  { "Match" { "Condition" { "User" = "foo" } }
    { "Settings" { "AllowGroups" { "1" = "users" } } } }

(* Test: Sshd.lns
     Recognize quoted group names with spaces in AllowGroups and similar
     (Issue #477) *)
test Sshd.lns get "Match User foo
    AllowGroups math-domain-users \"access admins\"\n" =
  { "Match" { "Condition" { "User" = "foo" } }
    { "Settings"
      { "AllowGroups"
        { "1" = "math-domain-users" }
        { "2" = "access admins" } } } }

test Sshd.lns put "Match User foo\nAllowGroups users\n" after
  set "/Match/Settings/AllowGroups/1" "all people" =
    "Match User foo\nAllowGroups \"all people\"\n"

test Sshd.lns put "Match User foo\nAllowGroups users\n" after
  set "/Match/Settings/AllowGroups/01" "all people" =
    "Match User foo\nAllowGroups users \"all people\"\n"

test Sshd.lns put "Match User foo\nAllowGroups users\n" after
  set "/Match/Settings/AllowGroups/01" "people" =
    "Match User foo\nAllowGroups users people\n"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
