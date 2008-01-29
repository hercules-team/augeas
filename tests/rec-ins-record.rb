commands="
ins /system/config/pam/newrole/10000 /system/config/pam/newrole/2
set /system/config/pam/newrole/10000/type test
set /system/config/pam/newrole/10000/control include
set /system/config/pam/newrole/10000/module system-auth
save
"
 
diff["/etc/pam.d/newrole"] = <<TXT
--- /etc/pam.d/newrole
+++ /etc/pam.d/newrole.augnew
@@ -1,5 +1,6 @@
 #%PAM-1.0
 auth       include\tsystem-auth
 account    include\tsystem-auth
+test\tinclude\tsystem-auth
 password   include\tsystem-auth
 session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close
TXT
