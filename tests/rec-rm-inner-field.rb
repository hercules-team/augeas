# FIXME: Ultimately, this test should fail since it leads
# to a file that is not syntactically correct
commands="
rm /system/config/pam/newrole/1/control
save
"

diff = {}
diff["/etc/pam.d/newrole"] = <<TXT
--- /etc/pam.d/newrole
+++ /etc/pam.d/newrole.augnew
@@ -1,5 +1,5 @@
 #%PAM-1.0
 auth       include\tsystem-auth
-account    include\tsystem-auth
+account    system-auth
 password   include\tsystem-auth
 session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close
TXT
