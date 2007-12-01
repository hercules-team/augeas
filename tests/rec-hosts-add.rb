# Note we don't set the aliases field here
commands="
ins /system/config/hosts//i0 /system/config/hosts/1
set /system/config/hosts/i0/ipaddr 192.168.0.1
set /system/config/hosts/i0/canonical pigiron.example.com
print
save
"
 
diff["/etc/hosts"] = <<TXT
--- /tmp/aug/etc/hosts
+++ /tmp/aug/etc/hosts.augnew
@@ -4,3 +4,4 @@
 #172.31.122.254   granny.watzmann.net granny puppet
 #172.31.122.1     galia.watzmann.net galia
 172.31.122.14   orange.watzmann.net orange
+192.168.0.1\tpigiron.example.com
TXT
