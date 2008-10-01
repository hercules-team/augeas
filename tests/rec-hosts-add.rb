# Note that aliases must be set (to NULL here), otherwise the tree
# is not syntactically correct and can not be saved
commands="
ins 10000 before /files/etc/hosts/2
set /files/etc/hosts/10000/ipaddr 192.168.0.1
set /files/etc/hosts/10000/canonical pigiron.example.com
save
"
refresh=true
diff["/etc/hosts"] = <<TXT
--- /etc/hosts
+++ /etc/hosts.augnew
@@ -3,4 +3,5 @@
 127.0.0.1\tlocalhost.localdomain\tlocalhost galia.watzmann.net galia
 #172.31.122.254   granny.watzmann.net granny puppet
 #172.31.122.1     galia.watzmann.net galia
+192.168.0.1\tpigiron.example.com
 172.31.122.14   orange.watzmann.net orange
TXT
