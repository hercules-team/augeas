# Convenience to save some typing
entry_last = "/files/etc/pam.d/newrole/10000"
commands="
ins entry after #{entry_last}
set #{entry_last}/type auth
set #{entry_last}/control include
set #{entry_last}/module system-auth
save
"

diff["/etc/pam.d/newrole"] = <<TXT
--- /etc/pam.d/newrole
+++ /etc/pam.d/newrole.augnew
@@ -3,3 +3,4 @@
 account    include\tsystem-auth
 password   include\tsystem-auth
 session    required\tpam_namespace.so unmnt_remnt no_unmount_on_close
+auth\tinclude\tsystem-auth
TXT
