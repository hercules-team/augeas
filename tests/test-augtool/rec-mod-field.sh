commands="
set /files/etc/pam.d/newrole/3/module other_module
set /files/etc/pam.d/newrole/3/argument other_module_opts
"

lens=Pam.lns
file="/etc/pam.d/newrole"

diff='--- /etc/pam.d/newrole
+++ /etc/pam.d/newrole.augnew
@@ -1,5 +1,5 @@
 #%PAM-1.0
 auth       include\tsystem-auth
 account    include\tsystem-auth
-password   include\tsystem-auth
+password   include\tother_module\tother_module_opts
 session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close'
