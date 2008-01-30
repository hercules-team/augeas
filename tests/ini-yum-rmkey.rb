commands="
rm /system/config/yum/main/keepcache
save
"
 
diff["/etc/yum.conf"] = <<TXT
--- /etc/yum.conf
+++ /etc/yum.conf.augnew
@@ -1,8 +1,7 @@
 # Test for yum
 [main]
 cachedir=/var/cache/yum
-keepcache=0
 debuglevel=2
-# some errant comment
 installonly_limit=100
+# some errant comment
 #and some blather at the end
TXT
