commands="
set /files/etc/yum.conf/main/debuglevel 99
"

lens=Yum.lns
file="/etc/yum.conf"

diff='--- /etc/yum.conf
+++ /etc/yum.conf.augnew
@@ -1,7 +1,7 @@
 [main]
 cachedir=/var/cache/yum
 keepcache=0
-debuglevel=2
+debuglevel=99
 logfile=/var/log/yum.log
 exactarch=1
 obsoletes=1'
