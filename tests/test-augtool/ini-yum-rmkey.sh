commands="
rm /files/etc/yum.conf/main/keepcache
"

lens=Yum.lns
file="/etc/yum.conf"

diff='--- /etc/yum.conf
+++ /etc/yum.conf.augnew
@@ -1,6 +1,5 @@
 [main]
 cachedir=/var/cache/yum
-keepcache=0
 debuglevel=2
 logfile=/var/log/yum.log
 exactarch=1'
