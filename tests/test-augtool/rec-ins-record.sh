commands="
ins 10000 before /files/etc/pam.d/newrole/3
set /files/etc/pam.d/newrole/10000/type session
set /files/etc/pam.d/newrole/10000/control include
set /files/etc/pam.d/newrole/10000/module system-auth
"

lens=Pam.lns
file="/etc/pam.d/newrole"

diff='--- /etc/pam.d/newrole
+++ /etc/pam.d/newrole.augnew
@@ -1,5 +1,6 @@
 #%PAM-1.0
 auth       include\tsystem-auth
 account    include\tsystem-auth
+session\tinclude\tsystem-auth
 password   include\tsystem-auth
 session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close'
