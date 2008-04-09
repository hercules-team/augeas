commands="
set /files/etc/pam.d/newrole/10000/type test
set /files/etc/pam.d/newrole/10000/control include
set /files/etc/pam.d/newrole/10000/module system-auth
save
"
 
diff["/etc/pam.d/newrole"] = <<TXT
--- /etc/pam.d/newrole
+++ /etc/pam.d/newrole.augnew
@@ -3,3 +3,4 @@
 account    include\tsystem-auth
 password   include\tsystem-auth
 session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close
+test\tinclude\tsystem-auth
TXT
