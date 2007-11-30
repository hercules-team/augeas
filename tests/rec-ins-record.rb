commands="
ins /system/config/pam/newrole/i0 /system/config/pam/newrole/2
set /system/config/pam/newrole/i0/type test
set /system/config/pam/newrole/i0/control include
set /system/config/pam/newrole/i0/module system-auth
save
"
 
diff["/etc/pam.d/newrole"] = <<TXT
--- /tmp/aug/etc/pam.d/newrole
+++ /tmp/aug/etc/pam.d/newrole.augnew
@@ -1,5 +1,6 @@
 #%PAM-1.0
 auth       include\tsystem-auth
 account    include\tsystem-auth
+test\tinclude\tsystem-auth
 password   include\tsystem-auth
 session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close
TXT
