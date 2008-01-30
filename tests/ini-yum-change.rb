commands="
set /system/config/yum/main/debuglevel 99
save
"
 
diff["/etc/yum.conf"] = <<TXT
--- /etc/yum.conf
+++ /etc/yum.conf.augnew
@@ -2,7 +2,7 @@
 [main]
 cachedir=/var/cache/yum
 keepcache=0
-debuglevel=2
+debuglevel=99
 # some errant comment
 installonly_limit=100
 #and some blather at the end
TXT
