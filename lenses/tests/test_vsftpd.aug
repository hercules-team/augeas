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


let conf = "# Example config file /etc/vsftpd/vsftpd.conf
#
# The default compiled in settings are fairly paranoid. This sample file
# loosens things up a bit, to make the ftp daemon more usable.
# Please see vsftpd.conf.5 for all compiled in defaults.
#
# Allow anonymous FTP? (Beware - allowed by default if you comment this out).
anonymous_enable=YES
#
# Default umask for local users is 077. You may wish to change this to 022,
# if your users expect that (022 is used by most other ftpd's)
local_umask=022
#
# You may specify an explicit list of local users to chroot() to their home
# directory. If chroot_local_user is YES, then this list becomes a list of
# users to NOT chroot().
chroot_list_enable=YES
# (default follows)
chroot_list_file=/etc/vsftpd/chroot_list
#

pam_service_name=vsftpd
userlist_enable=YES
tcp_wrappers=YES

"

test Vsftpd.lns get conf =
  { "#comment" = "Example config file /etc/vsftpd/vsftpd.conf" }
  {}
  { "#comment" = "The default compiled in settings are fairly paranoid. This sample file" }
  { "#comment" = "loosens things up a bit, to make the ftp daemon more usable." }
  { "#comment" = "Please see vsftpd.conf.5 for all compiled in defaults." }
  {}
  { "#comment" = "Allow anonymous FTP? (Beware - allowed by default if you comment this out)." }
  { "anonymous_enable" = "YES" }
  {}
  { "#comment" = "Default umask for local users is 077. You may wish to change this to 022," }
  { "#comment" = "if your users expect that (022 is used by most other ftpd's)" }
  { "local_umask" = "022" }
  {}
  { "#comment" = "You may specify an explicit list of local users to chroot() to their home" }
  { "#comment" = "directory. If chroot_local_user is YES, then this list becomes a list of" }
  { "#comment" = "users to NOT chroot()." }
  { "chroot_list_enable" = "YES" }
  { "#comment" = "(default follows)" }
  { "chroot_list_file" = "/etc/vsftpd/chroot_list" }
  {}
  {}
  { "pam_service_name" = "vsftpd" }
  { "userlist_enable" = "YES" }
  { "tcp_wrappers" = "YES" }
  {}

