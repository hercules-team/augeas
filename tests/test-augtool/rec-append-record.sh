commands='
ins 01 after /files/etc/pam.d/newrole/*[last()]
defvar new /files/etc/pam.d/newrole/*[last()]
set $new/type auth
set $new/control include
set $new/module system-auth
'

lens=Pam.lns
file="/etc/pam.d/newrole"

diff='--- /etc/pam.d/newrole
+++ /etc/pam.d/newrole.augnew
@@ -3,3 +3,4 @@
 account    include\tsystem-auth
 password   include\tsystem-auth
 session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close
+auth\tinclude\tsystem-auth'
