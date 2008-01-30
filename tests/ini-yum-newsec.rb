commands="
set /system/config/yum/other/newparam newval
save
"
 
diff["/etc/yum.conf"] = <<TXT
--- /etc/yum.conf
+++ /etc/yum.conf.augnew
@@ -6,3 +6,5 @@
 # some errant comment
 installonly_limit=100
 #and some blather at the end
+[other]
+newparam=newval
TXT
