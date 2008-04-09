commands="
set /files/etc/yum.conf/main/debuglevel 99
save
"

diff["/etc/yum.conf"] = <<TXT
--- /etc/yum.conf
+++ /etc/yum.conf.augnew
@@ -1,7 +1,7 @@
 [main]
 cachedir=/var/cache/yum
 keepcache=0
-debuglevel=2
+debuglevel=99
 logfile=/var/log/yum.log
 exactarch=1
 obsoletes=1
TXT
