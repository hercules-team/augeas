# Delete the first hosts entry and then add it back into the tree
# Because of how nodes are ordered, this should be equivalent to
# moving the corresponding line to the end of the file
refresh=true
commands="
rm /system/config/hosts/0
set /system/config/hosts/0/ipaddr 127.0.0.1
set /system/config/hosts/0/canonical localhost.localdomain
set /system/config/hosts/0/aliases/0 localhost
set /system/config/hosts/0/aliases/1 galia.watzmann.net
set /system/config/hosts/0/aliases/2 galia
save
"

diff["/etc/hosts"] = <<TXT
--- /etc/hosts
+++ /etc/hosts.augnew
@@ -1,6 +1,6 @@
 # Do not remove the following line, or various programs
 # that require network functionality will fail.
-127.0.0.1\tlocalhost.localdomain\tlocalhost galia.watzmann.net galia
 #172.31.122.254   granny.watzmann.net granny puppet
 #172.31.122.1     galia.watzmann.net galia
 172.31.122.14   orange.watzmann.net orange
+127.0.0.1\tlocalhost.localdomain localhost galia.watzmann.net galia
TXT
