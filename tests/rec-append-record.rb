commands="
set /system/config/pam/newrole/i0/type test
set /system/config/pam/newrole/i0/control include
set /system/config/pam/newrole/i0/module system-auth
save
"
 
diff["/etc/pam.d/newrole"] = <<TXT
--- /tmp/aug/etc/pam.d/newrole
+++ /tmp/aug/etc/pam.d/newrole.augnew
@@ -3,3 +3,4 @@
 account    include\tsystem-auth
 password   include\tsystem-auth
 session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close
+test\tinclude\tsystem-auth
TXT
