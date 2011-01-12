commands="
rm /files/etc/pam.d/newrole/4/argument
"

lens=Pam.lns
file="/etc/pam.d/newrole"

diff='--- /etc/pam.d/newrole
+++ /etc/pam.d/newrole.augnew
@@ -2,4 +2,4 @@
 auth       include\tsystem-auth
 account    include\tsystem-auth
 password   include\tsystem-auth
-session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close
+session    required\tpam_namespace.so'
