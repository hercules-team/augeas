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



  let match_blocks = "X11Forwarding yes
Match User sarko Group pres.*
  Banner /etc/bienvenue.txt
  X11Forwarding no
Match User bush Group pres.* Host white.house.*
Banner /etc/welcome.txt
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

  test Sshd.lns put match_blocks after
    insb "Subsystem" "/Match[1]";
    set "/Subsystem/sftp" "/usr/libexec/openssh/sftp-server"
  = "X11Forwarding yes
Subsystem sftp /usr/libexec/openssh/sftp-server
Match User sarko Group pres.*
  Banner /etc/bienvenue.txt
  X11Forwarding no
Match User bush Group pres.* Host white.house.*
Banner /etc/welcome.txt\n"


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
