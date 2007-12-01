# This will leave /etc/hosts with nothing but comments and whitespace
commands="
rm /system/config/hosts
save
"

diff["/etc/hosts"] = <<TXT
--- /tmp/aug/etc/hosts
+++ /tmp/aug/etc/hosts.augnew
@@ -1,6 +1,6 @@
 # Do not remove the following line, or various programs
 # that require network functionality will fail.
-127.0.0.1\tlocalhost.localdomain\tlocalhost galia.watzmann.net galia
+
 #172.31.122.254   granny.watzmann.net granny puppet
 #172.31.122.1     galia.watzmann.net galia
-172.31.122.14   orange.watzmann.net orange
+
TXT

