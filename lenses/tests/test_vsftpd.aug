module Test_vsftpd =

test Vsftpd.lns get "listen=YES\nmdtm_write=false\n" = 
  { "listen" = "YES" }
  { "mdtm_write" = "false" }

test Vsftpd.lns get "listen=on\n" = *

test Vsftpd.lns get "local_umask=0777\n" = { "local_umask" = "0777" }

test Vsftpd.lns get "listen_port=ftp\n" = *

test Vsftpd.lns get "ftp_username=ftp_user\n" = { "ftp_username" = "ftp_user" }

(* There must not be spaces around the '=' *)
test Vsftpd.lns get "anon_root = /var/lib/vsftpd/anon" = *




