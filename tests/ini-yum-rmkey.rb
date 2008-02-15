commands="
rm /system/config/yum/main/keepcache
save
"
 
diff["/etc/yum.conf"] = <<TXT
--- /etc/yum.conf
+++ /etc/yum.conf.augnew
@@ -1,7 +1,6 @@
 # Test for yum
 [main]
 cachedir=/var/cache/yum
-keepcache=0
 debuglevel=2
 # some errant comment
 installonly_limit=100
TXT
