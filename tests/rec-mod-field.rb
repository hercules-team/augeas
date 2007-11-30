commands="
set /system/config/pam/newrole/2/module other_module
set /system/config/pam/newrole/2/opts other_module_opts
save
"
    
diff["/etc/pam.d/newrole"] = <<TXT
--- /tmp/aug/etc/pam.d/newrole
+++ /tmp/aug/etc/pam.d/newrole.augnew
@@ -1,5 +1,5 @@
 #%PAM-1.0
 auth       include\tsystem-auth
 account    include\tsystem-auth
-password   include\tsystem-auth
+password   include\tother_module\tother_module_opts
 session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close
TXT
