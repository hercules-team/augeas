# -*- ruby -*-

commands="
rm /system/config/pam/newrole/1/control
save
"

diff = {}
diff["/etc/pam.d/newrole"] = <<TXT
--- /tmp/aug/etc/pam.d/newrole
+++ /tmp/aug/etc/pam.d/newrole.augnew
@@ -1,5 +1,5 @@
 #%PAM-1.0
 auth       include\tsystem-auth
-account    include\tsystem-auth
+account    system-auth
 password   include\tsystem-auth
 session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close
TXT
