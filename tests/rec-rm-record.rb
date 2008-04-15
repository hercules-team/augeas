# -*- ruby -*-
commands="
rm /files/etc/pam.d/newrole/1
save
"

diff["/etc/pam.d/newrole"] = <<TXT
--- /etc/pam.d/newrole
+++ /etc/pam.d/newrole.augnew
@@ -1,5 +1,4 @@
 #%PAM-1.0
-auth       include\tsystem-auth
 account    include\tsystem-auth
 password   include\tsystem-auth
 session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close
TXT
