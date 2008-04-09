commands="
rm /files/etc/yum.conf/main/keepcache
save
"

diff["/etc/yum.conf"] = <<TXT
--- /etc/yum.conf
+++ /etc/yum.conf.augnew
@@ -1,6 +1,5 @@
 [main]
 cachedir=/var/cache/yum
-keepcache=0
 debuglevel=2
 logfile=/var/log/yum.log
 exactarch=1
TXT
